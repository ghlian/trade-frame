/************************************************************************
 * Copyright(c) 2009, One Unified. All rights reserved.                 *
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
#include "TradingEnumerations.h"

namespace InstrumentType {
  const char *Name[] = { "Unknown", "Stock", "Option", "Future", 
    "FuturesOption", "Currency", "Index", "ETF", "Bond", "Commodity", "Metal",  };
}

namespace OptionSide {
  const char *Name[] = { "Unknown", "Put", "Call" };
  const char *LongName[] = { "Unknown", "Put", "Call" };
  const char *ShortName[] = { "U", "P", "C" };
}

namespace Currency {
  const char *Name[] = { "USD", "GBP", "CAD", "CHF", "HKD", "JPY", "EUR", "KRW", "LTL", "AUD", "CYP", "CZK", "DKK", "EEK", "NZD", "HUF", "ILS" };
}

namespace OrderSide {
  const char *Name[] = { "UNKN", "BUY", "SELL", "SSHORT" }; // for IB
}
