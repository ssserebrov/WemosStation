#include <Utilities.h>

String Utilities::floatToString(float floatVar, int numVarsAfterDecimal)
{
    static char outstr[15];
    dtostrf(floatVar, 7, numVarsAfterDecimal, outstr);
    String result(outstr);
    result.trim();
    return result;
}