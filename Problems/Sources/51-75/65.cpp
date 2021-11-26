#include "pch.h"
#include "51-75/65.h"
#include "bigNum.h"

/*
    The simple continued fraction represention of e is:
    [2;1,2,1,1,4,1,1,6,1,1,8,...]

    We can use the coefficients, a(n), of the continued fraction to generate successive convergents with the following iterative equations:

        For the convergent p(n) / q(n):
        
        p(n) = a(n)*p(n-1) + p(n-2)
        q(n) = a(n)*q(n-1) + q(n-2)

    and starting values:
    
        p(-2) = 0, p(-1) = 1
        q(-2) = 1, q(-1) = 0
*/
unsigned sumOfNumeratorDigits(unsigned nthConvergentOfe) {
    bigNum an = 2;                      // coeff of e's continued fraction
    bigNum pn2 = 0, pn1 = 1, pn = an;   // numerators of convergents 
    bigNum qn2 = 1, qn1 = 0, qn = 1;    // denominators of convergents

    for (unsigned n = 1, k = 1; n <= nthConvergentOfe; n++) {
        pn = an*pn1 + pn2;
        qn = an*qn1 + qn2;
        pn2 = pn1;
        qn2 = qn1;
        pn1 = pn;
        qn1 = qn;
        if ((n+1) % 3 == 0) an = 2*k++;
        else an = 1;
    }

    // print convergent
    std::cout << "Convergent " << nthConvergentOfe << " of e:\n";
    pn.print();
    std::cout << " /\n";
    qn.print();
    std::cout << "\n";

    unsigned sum = 0;
    for (unsigned i = 0; i < pn.numDigits(); i++)
        sum += pn[i];

    return sum;
}
