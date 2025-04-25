
https://binance-docs.github.io/apidocs/delivery_testnet/en/#all-market-tickers-streams
===========================================================================================================
						<symbol>@aggTrade | aggTrade - The Aggregate Trade Streams push trade information that is aggregated for a single taker order every 100 milliseconds.
===========================================================================================================

{
	"e":"aggTrade",       # // Event type
	"E":1591261134288,    # // Event time
	"a":424951,           # // Aggregate trade ID
	"s":"BTCUSD_200626",  # // Symbol
	"p":"9643.5",         # // Price
	"q":"2",              # // Quantity
	"f":606073,           # // First trade ID
	"l":606073,           # // Last trade ID
	"T":1591261134199,    # // Trade time
	"m":false             # // Is the buyer the market maker?
}

===========================================================================================================
						<pair>@indexPrice OR <pair>@indexPrice@1s | Index Price Stream
===========================================================================================================

{
	"e": "indexPriceUpdate",  # // Event type
	"E": 1591261236000,       # // Event time
	"i": "BTCUSD",            # // Pair
	"p": "9636.57860000",     # // Index Price
}

===========================================================================================================
						<symbol>@markPrice OR <symbol>@markPrice@1s | Mark Price Stream
===========================================================================================================

{
	"e":"markPriceUpdate",  # // Event type
	"E":1596095725000,      # // Event time
	"s":"BTCUSD_201225",    # // Symbol
	"p":"10934.62615417",   # // Mark Price
	"P":"10962.17178236",   # // Estimated Settle Price, only useful in the last hour before the settlement starts.
	"r":"",                 # // funding rate for perpetual symbol, "" will be shown for delivery symbol
	"T":0                   # // next funding time for perpetual symbol, 0 will be shown for delivery symbol
}

===========================================================================================================
						<pair>@markPrice OR <pair>@markPrice@1s  | Mark Price of All Symbols of a Pair
===========================================================================================================

	[
	  {
	    "e":"markPriceUpdate",  // Event type
	    "E":1596095725000,      // Event time
	    "s":"BTCUSD_201225",    // Symbol
	    "p":"10934.62615417",   // Mark Price
	    "P":"10962.17178236",    // Estimated Settle Price, only useful in the last hour before the settlement starts.
	    "r":"",                 // funding rate for perpetual symbol, "" will be shown for delivery symbol
	    "T":0                       // next funding time for perpetual symbol, 0 will be shown for delivery symbol
	  },
	  {
	    "e":"markPriceUpdate",
	    "E":1596095725000,
	    "s":"BTCUSD_PERP",
	    "p":"11012.31359011",
	    "P":"10962.17178236",
	    "r":"0.00000000",
	    "T":1596096000000
	  }
	]

===========================================================================================================
						<symbol>@ticker | Individual Symbol Ticker Streams
===========================================================================================================

{
  "e":"24hrTicker",             // Event type
  "E":1591268262453,            // Event time
  "s":"BTCUSD_200626",          // Symbol
  "ps":"BTCUSD",                // Pair
  "p":"-43.4",                  // Price change
  "P":"-0.452",                 // Price change percent
  "w":"0.00147974",             // Weighted average price
  "c":"9548.5",                 // Last price
  "Q":"2",                      // Last quantity
  "o":"9591.9",                 // Open price
  "h":"10000.0",                // High price
  "l":"7000.0",                 // Low price
  "v":"487850",                 // Total traded volume
  "q":"32968676323.46222700",   // Total traded base asset volume
  "O":1591181820000,            // Statistics open time
  "C":1591268262442,            // Statistics close time
  "F":512014,                   // First trade ID
  "L":615289,                   // Last trade Id
  "n":103272                    // Total number of trades
}


{
  "data": {
    "A": "0.08737000",
    "B": "0.10420000",
    "C": 1745472821988,
    "E": 1745472822004,
    "F": 1798873,
    "L": 1893166,
    "O": 1745386421988,
    "P": "-0.886",
    "Q": "0.00026000",
    "a": "92671.21000000",
    "b": "92671.20000000",
    "c": "92671.20000000",
    "e": "24hrTicker",
    "h": "120000.00000000",
    "l": "22009.80000000",
    "n": 94294,
    "o": "93499.99000000",
    "p": "-828.79000000",
    "q": "216672881.48598570",
    "s": "BTCUSDT",
    "v": "2319.45099000",
    "w": "93415.58947360",
    "x": "93499.99000000"
  },
  "stream": "btcusdt@ticker"
}

===========================================================================================================
						<symbol>@miniTicker | Individual Symbol Mini Ticker Stream
===========================================================================================================

# 24hr rolling window mini-ticker statistics for a single symbol. 
# These are NOT the statistics of the UTC day, but a 24hr rolling window from requestTime to 24hrs before.

{
  "e":"24hrMiniTicker",         // Event type
  "E":1591267704450,            // Event time
  "s":"BTCUSD_200626",          // Symbol
  "ps":"BTCUSD",                // Pair
  "c":"9561.7",                 // Close price
  "o":"9580.9",                 // Open price
  "h":"10000.0",                // High price
  "l":"7000.0",                 // Low price
  "v":"487476",                 // Total traded volume
  "q":"33264343847.22378500"    // Total traded base asset volume
}

===========================================================================================================
						<symbol>@bookTicker | Individual Symbol Book Ticker Streams
===========================================================================================================

# Pushes any update to the best bid or ask's price or quantity in real-time for a specified symbol.

{
  "e":"bookTicker",         // Event type
  "u":17242169,             // Order book update Id
  "s":"BTCUSD_200626",      // Symbol
  "ps":"BTCUSD",            // Pair
  "b":"9548.1",             // Best bid price
  "B":"52",                 // Best bid qty
  "a":"9548.5",             // Best ask price
  "A":"11",                 // Best ask qty
  "T":1591268628155,        // Transaction time
  "E":1591268628166         // Event time
}