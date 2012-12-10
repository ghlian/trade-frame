/************************************************************************
 * Copyright(c) 2012, One Unified. All rights reserved.                 *
 * email: info@oneunified.net                                           *
 *                                                                      *
 * This file is provided as is WITHOUT ANY WARRANTY                     *
 *  without even the implied warranty of                                *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                *
 *                                                                      *
 * This software may not be used nor distributed without proper license *
 * agreement.                                                           *
 *                                                                      *
 * See the file LICENSE.txt for redistribution information.             *
 ************************************************************************/

#include "StdAfx.h"

#include <boost/foreach.hpp>

#include <OUCommon/TimeSource.h>

#include "ManagePortfolio.h"

ManagePortfolio::ManagePortfolio(void)
  : m_dblPortfolioCashToTrade( 100000.0 ), m_dblPortfolioMargin( 0.15 )
{

  std::stringstream ss;
  ss.str( "" );
  ss << ou::TimeSource::Instance().External();
  m_sTSDataStreamStarted = "/app/BasketTrading/" + ss.str();  // will need to make this generic if need some for multiple providers.

}

ManagePortfolio::~ManagePortfolio(void) {
  BOOST_FOREACH( mapPositions_pair_t pair, m_mapPositions ) {
    delete pair.second;
  }
}

void ManagePortfolio::Start( pProvider_t pExec, pProvider_t pData1, pProvider_t pData2 ) {

  assert( 0 != m_mapPositions.size() );

  m_pExec = pExec;
  m_pData1 = pData1;
  m_pData2 = pData2;

  // first pass: to get rough idea of which can be traded given our funding level
  double dblAmountToTradePerInstrument = ( m_dblPortfolioCashToTrade / m_dblPortfolioMargin ) / m_mapPositions.size();

  unsigned int cntToBeTraded = 0;
  BOOST_FOREACH( mapPositions_pair_t pair, m_mapPositions ) {
    if ( 200 <= pair.second->CalcShareCount( dblAmountToTradePerInstrument ) ) {
      cntToBeTraded++;
      pair.second->ToBeTraded() = true;
    }
    else {
      pair.second->ToBeTraded() = false;
    }
  }

  // second pass: start trading with the ones that we can
  dblAmountToTradePerInstrument = ( m_dblPortfolioCashToTrade / m_dblPortfolioMargin ) / cntToBeTraded;
  BOOST_FOREACH( mapPositions_pair_t pair, m_mapPositions ) {
    if ( pair.second->ToBeTraded() ) {
      pair.second->Start( dblAmountToTradePerInstrument );
    }
  }
}

void ManagePortfolio::AddSymbol( const std::string& sName, const ou::tf::Bar& bar ) {
  assert( m_mapPositions.end() == m_mapPositions.find( sName ) );
  m_mapPositions[ sName ] = new Position( sName, bar );
}

void ManagePortfolio::SaveSeries( const std::string& sPath ) {
  BOOST_FOREACH( mapPositions_pair_t pair, m_mapPositions ) {
    pair.second->SaveSeries( sPath );
  }
}
