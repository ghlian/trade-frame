#include "StdAfx.h"

#include "IQFeedSymbolFile.h"
#include <fstream>
#include <iostream>
#include <map>
#include <boost/regex.hpp> 

#include "KeywordMatch.h"

using namespace std;

//	"select symbol from iqSymbols where "  
	//		+ " exchange in ('HOTSPOT', 'ONECH', 'NYSE', 'AMEX', 'NMS', 'CBOT', 'CME', 'DTN' )"
			// NASDAQ
			//+ " ( exchange='HOTSPOT'"
			//+ " or exchange='NASDAQ' or exchange='NYSE' or exchange='AMEX' or exchange='NMS' "
			//+ " or exchange='DTN' )"  // having a few problems with DTN (try again sometime)
//			+ " and ismutualfund = 0 and ismoneymarketfund = 0 and symbol not like 'RFC.%'"  // RFC causes abort in iqconnect

//select symbol, exchange, descr from iqsymbols where exchange in ('NMS', 'NYSE', 'AMEX')
	//			and 0=isindex and 0=iscboeindex and 0=ismutualfund and 0=ismoneymarketfund";


// symbolstats

// determine which are optionable symbols

//		string sCmd = "select distinct a.symbol from iqrootoptionsymbols as a, iqsymbols as b"
//			+ " where a.symbol=b.symbol and b.isindex=0 and b.iscboeindex=0 and b.isindicator=0";

// http://www.dtniq.com/product/mktsymbols.zip

// need a mechanism to detect lock files and then to eliminate them

CIQFeedSymbolFile::structExchangeInfo CIQFeedSymbolFile::m_rExchanges[] = {
  { "Unknown", InstrumentType::Unknown, 0 },
  { "AMEX", InstrumentType::Stock, 0 },
  { "BARCLAYS", InstrumentType::Currency, 0 },
  { "CBOT", InstrumentType::Future, 0 },
  { "CHX", InstrumentType::Future, 0 },
  { "CME", InstrumentType::Future, 0 },
  { "COMEX", InstrumentType::Future, 0 },
  { "CVE", InstrumentType::Stock, 0 },
  { "DJ", InstrumentType::Future, 0 },
  { "DTN", InstrumentType::Index, 0 },
  { "ENCOM", InstrumentType::Future, 0 },
  { "ENID", InstrumentType::Future, 0 },
  { "ENIR", InstrumentType::Future, 0 },
  { "EUREX", InstrumentType::Future, 0 },
  { "EUREXNDX", InstrumentType::Currency, 0 },
  { "IPE", InstrumentType::Future, 0 },
  { "KCBOT", InstrumentType::Future, 0 },
  { "LME", InstrumentType::Metal, 0 },
  { "MGE", InstrumentType::Future, 0 },
  { "NASDAQ", InstrumentType::Stock, 0 },
  { "NMS", InstrumentType::Stock, 0 },
  { "NYBOT", InstrumentType::Future, 0 },
  { "NYMEX", InstrumentType::Future, 0 },
  { "NYMEXMINI", InstrumentType::Future, 0 },
  { "NYSE", InstrumentType::Stock, 0 },
  { "ONECH", InstrumentType::Future, 0 },
  { "OPRA", InstrumentType::Option, 0 },
  { "OTC", InstrumentType::Stock, 0 },
  { "OTCBB", InstrumentType::Stock, 0 },
  { "PBOT", InstrumentType::Future, 0 },
  { "PSE", InstrumentType::Stock, 0 },
  { "SGX", InstrumentType::Future, 0 },
  { "SMCAP", InstrumentType::Stock, 0 },
  { "TSE", InstrumentType::Stock, 0 },
  { "TULLETT", InstrumentType::Currency, 0 },
  { "WCE", InstrumentType::Future, 0 }
};

CIQFeedSymbolFile::CIQFeedSymbolFile(void) : 
  CInstrumentFile( )
    {
}

CIQFeedSymbolFile::~CIQFeedSymbolFile(void) {
}

// http://www.relisoft.com/book/tech/6lib.html
class LessThan
    : public std::binary_function<char const *, char const *, bool> {
public:
    bool operator () (char const * str1, 
                      char const * str2) const {
        return strcmp (str1, str2) < 0;
    }
};

unsigned short DecodeMonth( const std::string &s ) {
  unsigned short month2 = 0;
  if ( 'J' < s[0] ) {
    if ( 'N' < s[0] ) {
      month2 = ( 'O' == s[0] ) ? 10 : 9;
    }
    else {
      if ( 'M' == s[0] ) {
        month2 = ( 'R' == s[2] ) ? 3 : 5;
      }
      else {
        month2 = 11;
      }
    }
  }
  else {
    if ( 'J' == s[0] ) {
      if ( 'U' == s[1] ) {
        month2 = ( 'N' == s[2] ) ? 6 : 7;
      }
      else {
        month2 = 1;
      }
    }
    else {
      if ( 'A' == s[0] ) {
        month2 = ( 'P' == s[1] ) ? 4 : 8;
      }
      else {
        month2 = ( 'D' == s[0] ) ? 12 : 2;
      }
    }
  }
  assert( month2 > 0 );
  return month2;
}

bool CIQFeedSymbolFile::Load( const string &filename ) {
  // mktsymbols.txt

  // TODO:  need to prevent re-entrant execution:  ie, turn invoking button off
  // TODO:  need to put in back ground thread, and lock out other access processes while running

  static const boost::regex rxMutual(".{3}[^\\.]X$");
  static const boost::regex rxMoneyMkt(".{3}XX$");
  static const boost::regex rxIndex(".+\\.X$");
  static const boost::regex rxCboe(".+\\.XO$");
  static const boost::regex rxIndicator(".+\\.Z$");
  static const boost::regex rxNotAStock("([A-Z]+[0-9]+[A-Z]*)|([A-Z]+\\.[A-Z]+)");

  static const boost::regex rxOption( "^([A-Z]+){0,1}[ ]([A-Z]{3}) ([0-9]{4}) ([CP]) ([0-9]+[.][0-9]+)" ); //GM SEP 2008 P 10.000
  static const boost::regex rxOptionName( "^[A-Z]{1,3} [A-Z][A-Z]$" );
  static const boost::regex rxFuture( "[[:blank:]](JAN|FEB|MAR|APR|MAY|JUN|JUL|AUG|SEP|OCT|NOV|DEC)[[:blank:]]{1}([0-9]{4})" );  // GOOG1CQ07	GOOG AUG 2007	ONECH
  static const boost::regex rxFuture2( "(JAN|FEB|MAR|APR|MAY|JUN|JUL|AUG|SEP|OCT|NOV|DEC)[[:blank:]][0-9]{2}/(JAN|FEB|MAR|APR|MAY|JUN|JUL|AUG|SEP|OCT|NOV|DEC)[[:blank:]][0-9]{2}" ); //     No future match on HOX8-HOJ9, HEATING OIL #2 NOV 08/APR 09

  unsigned int cntMutual = 0, cntMoneyMkt = 0, cntIndex = 0, cntCboe = 0, cntIndicator = 0, cntNotAStock = 0;
  unsigned short nUnderlyingSize = 0;

  ifstream file;
  unsigned long cntLines = 0;
  td_structIndexes j, k, c; 
  bool bEndFound;

  std::map<std::string, unsigned long> mapUnderlying;  // keeps track of optionable symbols, to fix bool at end

  cout << "Initializing Structures" << endl;

  CKeyWordMatch<unsigned long> kwm( 0, 300 );  // about 300 characters?

  for ( unsigned long ix = 0; ix < sizeof( m_rExchanges ) / sizeof( structExchangeInfo ); ++ix ) {
    m_rExchanges[ ix ].cntInstruments = 0;
    kwm.AddPattern( m_rExchanges[ ix ].szName, ix );
  }
  std::cout << "kvm size is " << kwm.size() << std::endl;
  unsigned long rcntContractTypes[ InstrumentType::_Count ];
  for ( unsigned long ix = 0; ix < InstrumentType::_Count; ++ix ) {
    rcntContractTypes[ ix ] = 0;
  }

  cout << "Opening Symbol Database" << endl;
  file.open( filename.c_str() );

  // use 'db_recover -v' tool if the database hangs here
  OpenIQFSymbols();
  u_int32_t countp = 0;
  cout << "Truncating Symbol Database" << endl;
  m_pdbSymbols->truncate( NULL, &countp, 0 );

  cout << "Loading Symbols" << endl;
  try {
    file.getline( dbRecord.line, nMaxBufferSize );  // remove header line
    file.getline( dbRecord.line, nMaxBufferSize );
    while ( !file.fail() ) {
      ++cntLines;
      bEndFound = false;
      c = j = k = 0; // c is count of char in string, j index into ix, k index into line
      dbRecord.ucBits1 = 0;  // initialize bits to zero
      UnPackBoolean( dbRecord.ucBits1 );
      dbRecord.fltStrike = 0;
      dbRecord.nYear = 0;
      dbRecord.nMonth = 0;
      dbRecord.nOptionSide = OptionSide::Unknown;
      dbRecord.ix[j] = k; // index of first string is 0
      while ( !bEndFound ) {
        if ( 0 == dbRecord.line[k] ) {
          dbRecord.cnt[j] = c; // size of last string
          bEndFound = true;
        }
        else {
          if ( '\t' != dbRecord.line[k] ) { 
            ++c;
          }
          else {
            dbRecord.cnt[j] = c;
            c = 0;
            ++j;
            if ( j >= 3 ) {
              bEndFound = true;
            }
            else {
              dbRecord.ix[j] = k+1; // beginning of new string
            }
            dbRecord.line[k] = 0;  //force a string end
          }
          ++k;
        }
      }
      dbRecord.bufferedlength = sizeof( structSymbolRecord ) - nMaxBufferSize + k + 1;  // +1 is for zero offset of k
      // process line here

      size_t ix;
      try {
        ix = (size_t) kwm.FindMatch( dbRecord.line + dbRecord.ix[2] );
        dbRecord.eInstrumentType = m_rExchanges[ ix ].eInstrumentType;
        ++m_rExchanges[ ix ].cntInstruments;
      }
      catch ( std::exception e ) {
        std::cout << dbRecord.line << ": zero length pattern" << std::endl;
        dbRecord.eInstrumentType = InstrumentType::Unknown;
        ++m_rExchanges[ 0 ].cntInstruments;
      }
      
      ++rcntContractTypes[ dbRecord.eInstrumentType ];
      if ( InstrumentType::Unknown == dbRecord.eInstrumentType ) {
        cout << "unknown contract: " << dbRecord.line << ", " << dbRecord.line + dbRecord.ix[2] << endl;
      }
      // parse out contract expiry information
      if ( InstrumentType::Future == dbRecord.eInstrumentType ) {
        boost::cmatch what;
        if ( boost::regex_search( dbRecord.line + dbRecord.ix[1], what, rxFuture ) ) {
          std::string sMonth( what[1].first, what[1].second );
          std::string sYear( what[2].first, what[2].second );
          dbRecord.nMonth = DecodeMonth( sMonth );
          dbRecord.nYear = atoi( sYear.c_str() );
        }
        else {  // No future match on HOX8-HOJ9, HEATING OIL #2 NOV 08/APR 09
          if ( boost::regex_search( dbRecord.line + dbRecord.ix[1], what, rxFuture2 ) ) {
            // just ignore the double future set
          }
          else {
            std::cout << "No future match on " << dbRecord.line << ", " << dbRecord.line + dbRecord.ix[1] << std::endl;
          }
        }
      }

      // parse out contract information
      if ( InstrumentType::Option == dbRecord.eInstrumentType ) {
        boost::cmatch what1, what2;
        if ( boost::regex_search( dbRecord.line + dbRecord.ix[1], what1, rxOption, boost::match_default )
          && boost::regex_search( dbRecord.line, what2, rxOptionName, boost::match_default )
          ) {
          std::string sUnderlying( what1[1].first, what1[1].second );
          if ( 0 == sUnderlying.size() ) {
            std::cout << "Zero length underlying on " << dbRecord.line << std::endl;
          }
          else {
            mapUnderlying[ sUnderlying ] = 1;  // simply create an entry for later use
          }
          nUnderlyingSize = max( nUnderlyingSize, sUnderlying.size() );
          std::string sMonth( what1[2].first, what1[2].second );
          std::string sYear( what1[3].first, what1[3].second );
          dbRecord.nOptionSide = OptionSide::Unknown;
          if ( 'P' == *what1[4].first ) dbRecord.nOptionSide = OptionSide::Put;
          if ( 'C' == *what1[4].first ) dbRecord.nOptionSide = OptionSide::Call;
          //dbRecord.chDirection = *what[4].first;
          std::string sStrike( what1[5].first, what1[5].second );
          dbRecord.nMonth = DecodeMonth( sMonth );
          dbRecord.nYear = atoi( sYear.c_str() );
          dbRecord.fltStrike = atof( sStrike.c_str() );
          
        }
        else {
          std::cout  << "No option match on " << dbRecord.line << ", " << dbRecord.line + dbRecord.ix[1] << std::endl;
        }
      }

      // identify the symbol types
      m_bMutual = m_bMoneyMkt = m_bIndex = m_bCboe = m_bIndicator , m_bNotAStock= false;
      if ( boost::regex_match( dbRecord.line, rxMutual ) ) {
        m_bMutual = true;
        ++cntMutual;
      }
      if ( boost::regex_match( dbRecord.line, rxMoneyMkt ) ) {
        m_bMoneyMkt = true;
        ++cntMoneyMkt;
      }
      if ( boost::regex_match( dbRecord.line, rxIndex ) ) {
        m_bIndex = true;
        ++cntIndex;
      }
      if ( boost::regex_match( dbRecord.line, rxCboe ) ) {
        m_bCboe = true;
        ++cntCboe;
      }
      if ( boost::regex_match( dbRecord.line, rxIndicator ) ) {
        m_bIndicator = true;
        ++cntIndicator;
      }
      if ( boost::regex_match( dbRecord.line, rxNotAStock ) ) {
        m_bNotAStock = true;
        ++cntNotAStock;
      }
      PackBoolean();

      // update database
      //Dbt key( dbRecord.line, dbRecord.cnt[0]+1 );
      Dbt key( dbRecord.line, dbRecord.cnt[0] );
      assert( dbRecord.bufferedlength <= 255 );
      Dbt value( &dbRecord, dbRecord.bufferedlength );
      int ret = m_pdbSymbols->put( 0, &key, &value, DB_NOOVERWRITE );
      // get next line from text file
      file.getline( dbRecord.line, nMaxBufferSize );
    }

    structSymbolRecord rec;
    Dbt k;
    Dbt v;
    v.set_flags( DB_DBT_USERMEM );
    v.set_data( &rec );  
    int ret;
    for ( std::map<std::string, unsigned long>::iterator iter = mapUnderlying.begin();
      mapUnderlying.end() != iter; ++iter ) {
      // iterate through map and update ucHasOptions flag in each record
      k.set_data( (void*) iter->first.c_str() );
      k.set_size( iter->first.size() );
      v.set_ulen( sizeof( structSymbolRecord ) );
      v.set_size( sizeof( structSymbolRecord ) );
      ret = m_pdbSymbols->get( 0, &k, &v, 0 );
      if ( 0 == ret ) {
        rec.ucBits1 |= ucHasOptions;  // set the bit
        k.set_data( (void*) iter->first.c_str() );
        k.set_size( iter->first.size() );
        ret = m_pdbSymbols->put( 0, &k, &v, 0 );
        if ( 0 != ret ) {
          std::cout << "failed write on " << iter->first << std::endl;
        }
      }
      else {
        std::cout << "failed read on " << iter->first << std::endl; // no underlying listed
      }

    }
  }
  catch( DbException &dbex ) {
    cout << "IQFSymbols exception: " << dbex.what() << endl;
  }

  CloseIQFSymbols();
  file.close();

  cout << "Count Mutual:     " << cntMutual << endl;
  cout << "Count MoneyMkt:   " << cntMoneyMkt << endl;
  cout << "Count Index:      " << cntIndex << endl;
  cout << "Count CBOE:       " << cntCboe << endl;
  cout << "Count Indicator:  " << cntIndicator << endl;
  cout << "Count NotAStock   " << cntNotAStock << endl;
  cout << "Count Optionables " << mapUnderlying.size() << endl;
  cout << "Max Underlying    " << nUnderlyingSize << endl;

  for ( unsigned long ix = 0; ix < sizeof( m_rExchanges ) / sizeof( structExchangeInfo ); ++ix ) {
    std::cout << m_rExchanges[ ix ].szName << "=" << m_rExchanges[ ix ].cntInstruments << std::endl;
  }
  std::cout << "Contract Types" << std::endl;
  for ( unsigned long ix = 0; ix < InstrumentType::_Count; ++ix ) {
    std::cout << ix << "=" << rcntContractTypes[ ix ] << std::endl;
  }

  return true;
}

