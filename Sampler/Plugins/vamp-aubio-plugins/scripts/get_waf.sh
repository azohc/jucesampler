#! /bin/sh

set -e
set -x

WAFURL=https://waf.io/waf-1.8.22

( which wget > /dev/null && wget -qO waf $WAFURL ) || ( which curl > /dev/null && curl $WAFURL > waf )

chmod +x waf
