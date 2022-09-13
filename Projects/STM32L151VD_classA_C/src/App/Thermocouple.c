/*******************************************************************************
* File Name: Thermocouple.c
*******************************************************************************/

#include "Thermocouple.h"

/***************************************
*        Constants
***************************************/
#define     THERMOCOUPLE_INIT                       (0)
#define     THERMOCOUPLE_FIRST_EL_MAS               (0u)
#define     THERMOCOUPLE_RANGE_MAS_0                (0u)
#define     THERMOCOUPLE_RANGE_MAS_1                (1u)
#define     THERMOCOUPLE_RANGE_MAS_2                (2u)
#define     THERMOCOUPLE_RANGE_MAS_3                (3u)
#define     THERMOCOUPLE_THREE                      (3u)
#define     THERMOCOUPLE_IN_NORMALIZATION_VT        (24)
#define     THERMOCOUPLE_IN_NORMALIZATION_TV        (24u)
#define     THERMOCOUPLE_24BIT_SHIFTING             (24u)
#define     THERMOCOUPLE_16BIT_SHIFTING             (16u)
#define     THERMOCOUPLE_8BIT_SHIFTING              (8u)
#define     THERMOCOUPLE_24BIT_CUTTING              (0xFFFFFFlu)
#define     THERMOCOUPLE_16BIT_CUTTING              (0xFFFFu)
#define     THERMOCOUPLE_8BIT_CUTTING               (0xFFu)
#define     THERMOCOUPLE_V_IN_FLOAT_NORMALIZATION   (1000u)
#define     THERMOCOUPLE_V_OUT_FLOAT_NORMALIZATION  (100u)
#define     THERMOCOUPLE_T_IN_FLOAT_NORMALIZATION   (100u)
#define     THERMOCOUPLE_T_OUT_FLOAT_NORMALIZATION  (1000u)


/***************************************
*  Customizer Generated Defines
***************************************/
#define     THERMOCOUPLE_X_SCALE_TV   (0x0Fu)
#define     THERMOCOUPLE_COEF_SCALE_TV   (0x05u)

#if(TC_CALC_FLOAT)  
const float aafThermocouple_coeffVT[][4] = { 
    {5593.483, 9842.332, 21315.07},                                 // B 0
    {161.8633, 69.9715, 28.5105},                                   // B 1
    {-1.058366, -0.08476531, -0.005274289},                         // B 2
    {0.006082131, 0.0001005264, 9.916081E-07},                      // B 3
    {-2.055976E-05, -8.334595E-08, -1.29653E-10},                   // B 4
    {3.177028E-08, 4.550854E-11, 1.119587E-14},                     // B 5
    {0, -1.552304E-14, -6.06252E-19},                               // B 6
    {-3.860831E-14, 2.988675E-18, 1.86617E-23},                     // B 7
    {0, -2.474286E-22, -2.487859E-28},                              // B 8
    {0,0,0,0},                                                      // B 9 
    {0,0,0,0},                                                      // B 10
    {0,0,0,0},                                                      // C 0 
    {0,0,0,0},                                                      // C 1 
    {0,0,0,0},                                                      // C 2 
    {0,0,0,0},                                                      // C 3 
    {0,0,0,0},                                                      // C 4 
    {0,0,0,0},                                                      // C 5 
    {0,0,0,0},                                                      // C 6 
    {0,0,0,0},                                                      // C 7 
    {0,0,0,0},                                                      // C 8                                              
    {0,0,0,0},                                                      // C 9
    {0,0,0,0},                                                      // C 10
    {0,0,0,0},                                                      // D 0 
    {0,0,0,0},                                                      // D 1 
    {0,0,0,0},                                                      // D 2 
    {0,0,0,0},                                                      // D 3 
    {0,0,0,0},                                                      // D 4 
    {0,0,0,0},                                                      // D 5 
    {0,0,0,0},                                                      // D 6 
    {0,0,0,0},                                                      // D 7 
    {0,0,0,0},                                                      // D 8                                              
    {0,0,0,0},                                                      // D 9
    {0,0,0,0},                                                      // D 10                                            
    {2789311, 0, -5.578945},                                        // E 0
    {0, 1.697729, 1.71236},                                         // E 1
    {0, -4.351497E-05, -2.52372E-05},                               // E 2
    {0, -1.58597E-08, 8.927976E-10},                                // E 3
    {0, -9.250287E-12, -2.267813E-14},                              // E 4
    {5.422079E-12, -2.608431E-15, 3.858364E-19},                    // E 5
    {1.969221E-15, -4.13602E-19, -3.976922E-24},                    // E 6
    {2.75876E-19, -3.403403E-23, 2.202885E-29},                     // E 7
    {1.753582E-23, -1.156489E-27, -4.918305E-35},                   // E 8
    {4.246546E-28, 0, 0},                                           // E 9
    {0,0,0,0},                                                      // E 10
    {6.738966, 5.758141, -307667.6},                                // J 0
    {2.045089, 1.974361, 36.69442},                                 // J 1
    {9.572693E-05, -1.910817E-05, -0.001746711},                    // J 2
    {9.725651E-08, 9.449809E-10, 5.14869E-08},                      // J 3
    {3.712949E-11, -2.060178E-14, -9.758174E-13},                   // J 4
    {7.613363E-15, 2.176919E-19, 1.17076E-17},                      // J 5
    {7.784742E-19, -3.27201E-24, -8.029483E-23},                    // J 6
    {3.203106E-23, 3.878452E-29, 2.377221E-28},                     // J 7
    {0,0,0,0},                                                      // J 8                                              
    {0,0,0,0},                                                      // J 9 
    {0,0,0,0},                                                      // J 10
    {0, 0, -13180.58},                                              // K 0
    {2.517346, 2.508355, 4.830222},                                 // K 1
    {-0.0001166288, 7.860106E-06, -0.0001646031},                   // K 2
    {-1.083364E-07, -2.503131E-08, 5.464731E-09},                   // K 3
    {-8.977354E-11, 8.31527E-12, -9.650715E-14},                    // K 4
    {-3.734238E-14, -1.228034E-15, 8.802193E-19},                   // K 5
    {-8.663264E-18, 9.804036E-20, -3.11081E-24},                    // K 6
    {-1.04506E-21, -4.41303E-24, 0},                                // K 7
    {-5.192058E-26, 1.057734E-28, 0},                               // K 8
    {0, -1.052755E-33, 0},                                          // K 9
    {0,0,0,0},                                                      // K 10
    {0,0,0,0},                                                      // L 0 
    {0,0,0,0},                                                      // L 1 
    {0,0,0,0},                                                      // L 2 
    {0,0,0,0},                                                      // L 3 
    {0,0,0,0},                                                      // L 4 
    {0,0,0,0},                                                      // L 5 
    {0,0,0,0},                                                      // L 6 
    {0,0,0,0},                                                      // L 7 
    {0,0,0,0},                                                      // L 8                                              
    {0,0,0,0},                                                      // L 9 
    {0,0,0,0},                                                      // L 10
    {1.121389E+07, 0, 0, 1972.485},                                 // N 0
    {0, 3.843685, 3.86896, 3.300943},                               // N 1
    {0, 0.0001101048, -0.000108267, -3.915159E-05},                 // N 2
    {0, 5.222931E-07, 4.70205E-09, 9.855391E-10},                   // N 3
    {0, 7.206052E-10, -2.12169E-16, -1.274371E-14},                 // N 4
    {1.208978E-09, 5.848859E-13, -1.17272E-17, 7.767022E-20},       // N 5
    {9.808906E-13, 2.775492E-16, 5.3928E-22, 0},                    // N 6
    {3.06963E-16, 7.707516E-20, -7.98156E-27, 0},                   // N 7
    {4.358352E-20, 1.158266E-23, 0, 0},                             // N 8
    {2.357392E-24, 7.313887E-28, 0, 0},                             // N 9
    {0,0,0,0},                                                      // N 10
    {0, 1334.584, -8199.6, 3406178},                                // R 0
    {18.89138, 14.72645, 15.53962, -702.3729},                      // R 1
    {-0.009383529, -0.001844025, -0.0008342197, 0.05582904},        // R 2
    {1.306862E-05, 4.03113E-07, 4.279434E-08, -1.952395E-06},       // R 3
    {-2.270358E-08, -6.249428E-11, -1.191578E-12, 2.56074E-11},     // R 4
    {3.514566E-11, 6.468412E-15, 1.49229E-17, 0},                   // R 5
    {-3.89539E-14, -4.458751E-19, 0, 0},                            // R 6
    {2.823947E-17, 1.99471E-23, 0, 0},                              // R 7
    {-1.260728E-20, -5.313402E-28, 0, 0},                           // R 8
    {3.135361E-24, 6.481976E-33, 0, 0},                             // R 9
    {-3.318777E-28, 0, 0, 0},                                       // R 10
    {0, 1291.507, -8087.801, 1269514},                              // S 0
    {18.49495, 14.66299, 16.21573, 0},                              // S 1
    {-0.008005041, -0.001534713, -0.0008536869, -0.03960271},       // S 2 
    {1.022374E-05, 3.145946E-07, 4.719687E-08, 4.627939E-06},       // S 3
    {-1.522486E-08, -4.163258E-11, -1.441694E-12, -2.013753E-10},   // S 4
    {1.888213E-11, 3.187964E-15, 2.081619E-17, 3.112701E-15},       // S 5
    {-1.590859E-14, -1.291637E-19, 0, 0},                           // S 6
    {8.230279E-18, 2.183475E-24, 0, 0},                             // S 7
    {-2.341819E-21, -1.44738E-29, 0, 0},                            // S 8
    {2.797863E-25, 8.211273E-34, 0, 0},                             // S 9
    {0,0,0,0},                                                      // S 10  
    {1.501584E+07, 2.085958, -5.101194},                            // T 0
    {0, 2.606901, 2.601227},                                        // T 1
    {0, 5.496804E-07, -8.038932E-05},                               // T 2
    {0.002676277, 9.68921E-08, 5.659293E-09},                       // T 3
    {1.380973E-06, 5.013887E-11, -3.406639E-13},                    // T 4
    {2.850472E-10, 1.506442E-14, 1.41527E-17},                      // T 5
    {2.724015E-14, 2.233777E-18, -3.424984E-22},                    // T 6
    {1.004136E-18, 1.367554E-22, 3.582326E-27},                     // T 7
    {0,0,0,0},                                                      // T 8                                              
    {0,0,0,0},                                                      // T 9 
    {0,0,0,0},                                                      // T 10  
    {0,0,0,0},                                                      // U 0 
    {0,0,0,0},                                                      // U 1 
    {0,0,0,0},                                                      // U 2 
    {0,0,0,0},                                                      // U 3 
    {0,0,0,0},                                                      // U 4 
    {0,0,0,0},                                                      // U 5 
    {0,0,0,0},                                                      // U 6 
    {0,0,0,0},                                                      // U 7 
    {0,0,0,0},                                                      // U 8                                              
    {0,0,0,0},                                                      // U 9 
    {0,0,0,0}                                                       // U 10                                              
};
#else
const int32_t aai32Thermocouple_coeffVT[][4]= {
    {44748, 1259819, 341041, 0},                  // B 0
    {1325984, 18342609, 7473858, 0},              // B 1
    {-8878215, -45508026, -22652897, 0},          // B 2
    {52245108, 110529990, 69778212, 0},           // B 3
    {-180845572, -187678399, -149479766, 0},      // B 4
    {286161260, 209871107, 211484005, 0},         // B 5
    {0, -146610939, -187625903, 0},               // B 6
    {-364645150, 57809382, 94626041, 0},          // B 7
    {0, -9801657, -20668320, 0},                  // B 8
    {0,0,0,0},                                    // B 9 
    {0,0,0,0},                                    // B 10
    {0,0,0,0},                                    // C 0 
    {0,0,0,0},                                    // C 1 
    {0,0,0,0},                                    // C 2 
    {0,0,0,0},                                    // C 3 
    {0,0,0,0},                                    // C 4 
    {0,0,0,0},                                    // C 5 
    {0,0,0,0},                                    // C 6 
    {0,0,0,0},                                    // C 7 
    {0,0,0,0},                                    // C 8                                              
    {0,0,0,0},                                    // C 9
    {0,0,0,0},                                    // C 10
    {0,0,0,0},                                    // D 0 
    {0,0,0,0},                                    // D 1 
    {0,0,0,0},                                    // D 2 
    {0,0,0,0},                                    // D 3 
    {0,0,0,0},                                    // D 4 
    {0,0,0,0},                                    // D 5 
    {0,0,0,0},                                    // D 6 
    {0,0,0,0},                                    // D 7 
    {0,0,0,0},                                    // D 8                                              
    {0,0,0,0},                                    // D 9
    {0,0,0,0},                                    // D 10                                            
    {89257945, 0, -89, 0},                        // E 0 
    {0, 890099, 3591080, 0},                      // E 1
    {0, -373791, -6937149, 0},                    // E 2
    {0, -2232054, 32166422, 0},                   // E 3
    {0, -21329710, -107094423, 0},                // E 4
    {200039420, -98543752, 238821140, 0},         // E 5
    {297580276, -256007232, -322646285, 0},       // E 6
    {170758969, -345146068, 234250955, 0},        // E 7
    {44458597, -192154694, -68551162, 0},         // E 8
    {4409865, 0, 0, 0},                           // E 9
    {0,0,0,0},                                    // E 10
    {6901, 2948, -4922681, 0},                    // J 0 
    {17155453, 66248577, 38476894, 0},            // J 1
    {6578305, -42019308, -120033069, 0},          // J 2
    {54750547, 136186099, 231876366, 0},          // J 3
    {171229557, -194578269, -288010449, 0},       // J 4
    {287624689, 134744790, 226457841, 0},         // J 5
    {240926101, -132728482, -101785785, 0},       // J 6
    {81208389, 103106924, 19749178, 0},           // J 7
    {0,0,0,0},                                    // J 8                                              
    {0,0,0,0},                                    // J 9 
    {0,0,0,0},                                    // J 10
    {0, 0, -1687114, 0},                          // K 0
    {2639629, 164388, 40518839, 0},               // K 1
    {-1001834, 16879, -90491511, 0},              // K 2
    {-7623495, -1761422, 196887684, 0},           // K 3
    {-51750922, 19173707, -227871065, 0},         // K 4
    {-176344390, -92787626, 136207339, 0},        // K 5
    {-335143805, 242736174, -31547361, 0},        // K 6
    {-331192671, -358027528, 0, 0},               // K 7
    {-134793524, 281193929, 0, 0},                // K 8
    {0, -91707895, 0, 0},                         // K 9
    {0,0,0,0},                                    // K 10
    {0,0,0,0},                                    // L 0 
    {0,0,0,0},                                    // L 1 
    {0,0,0,0},                                    // L 2 
    {0,0,0,0},                                    // L 3 
    {0,0,0,0},                                    // L 4 
    {0,0,0,0},                                    // L 5 
    {0,0,0,0},                                    // L 6 
    {0,0,0,0},                                    // L 7 
    {0,0,0,0},                                    // L 8                                              
    {0,0,0,0},                                    // L 9 
    {0,0,0,0},                                    // L 10
    {44855568, 0, 0, 4039649},                    // N 0
    {0, 2015198, 64910378, 221522535},            // N 1
    {0, 236448, -59520413, -86095257},            // N 2
    {0, 4594139, 84704603, 71015576},             // N 3
    {0, 25962540, -125242, -30090234},            // N 4
    {174232113, 86313918, -226837038,6009442},    // N 5   
    {289507809, 167768173, 341809308, 0},         // N 6
    {185547740, 190828868, -165770516, 0},        // N 7 
    {53953780, 117462178, 0, 0},                  // N 8
    {5976698, 30380697, 0, 0},                    // N 9
    {0,0,0,0},                                    // N 10
    {0, 42707, -8396390, 217995382},              // R 0
    {2476131, 7720899, 260711568, -184122846},    // R 1
    {-2518872, -15840053, -229308583, 59945973},  // R 2
    {7184549, 56733107, 192728553, -8586722},     // R 3
    {-25561959, -144102007, -87922931, 461302},   // R 4
    {81040372, 244369698, 18040680, 0},           // R 5
    {-183954592, -275983284, 0, 0},               // R 6
    {273115405, 202287641, 0, 0},                 // R 7
    {-249712927, -88284030, 0, 0},                // R 8
    {127185356, 17645618, 0, 0},                  // R 9
    {-27571320, 0, 0, 0},                         // R 10
    {0, 2583, -8281908, 324995551},               // S 0
    {9696678, 960954, 272054822, 0},              // S 1
    {-8595347, -3295772, -234659681, -170092341}, // S 2
    {22482249, 22137627, 212555805, 81415568},    // S 3
    {-68566670, -95998190, -106378216, -14510622},// S 4
    {174156949, 240875732, 25165228, 918707},     // S 5
    {-300504846, -319793955, 0, 0},               // S 6
    {318393489, 177144544, 0, 0},                 // S 7
    {-185538051, -38477947, 0, 0},                // S 8
    {45397916, 71530269, 0, 0},                   // S 9
    {0,0,0,0},                                    // S 10  
    {120126711, 0, 0, 0},                         // T 0
    {0, 10883880, 86999931, 0},                   // T 1
    {0, -732445, -83595440, 0},                   // T 2
    {183912329, 22241784, 167094031, 0},          // T 3
    {194354670, 98062377, -255644601, 0},         // T 4
    {82159257, 251314389, 233976238, 0},          // T 5
    {16079749, 313221206, -92455108, 0},          // T 6
    {1213926, 160588146, 0, 0},                   // T 7
    {0,0,0,0},                                    // T 8                                              
    {0,0,0,0},                                    // T 9 
    {0,0,0,0},                                    // T 10  
    {0,0,0,0},                                    // U 0 
    {0,0,0,0},                                    // U 1 
    {0,0,0,0},                                    // U 2 
    {0,0,0,0},                                    // U 3 
    {0,0,0,0},                                    // U 4 
    {0,0,0,0},                                    // U 5 
    {0,0,0,0},                                    // U 6 
    {0,0,0,0},                                    // U 7 
    {0,0,0,0},                                    // U 8                                              
    {0,0,0,0},                                    // U 9 
    {0,0,0,0}                                     // U 10                                              
};
#endif

#if(!TC_CALC_FLOAT)

    /*******************************************************************************
    * Function Name: Thermocouple_MultShift24
    ********************************************************************************
    *
    * Summary:
    *  Performs the math function (op1 * op2) >> 24 using 64 bit arithmetic without
    *  any loss of precision and without overflow.
    *
    * Parameters:
    *  op1: Signed 32-bit operand
    *  op2: Signed 24-bit operand
    *
    * Return:
    *  Signed 32-bit result of the math calculation
    *
    *******************************************************************************/
    int32_t Thermocouple_MultShift24(int32_t i32Op1, int32_t i32Op2) 
    {
        int64_t i64Result=0;

        i64Result = (int64_t)i32Op1 * (int64_t)i32Op2;
        if (i64Result < 0)
        {
            i64Result = -i64Result;
            i64Result = (int32_t)((uint64_t)((uint64_t)i64Result >> THERMOCOUPLE_24BIT_SHIFTING));
            i64Result = -i64Result;
        }
        else
        {
            i64Result = (int32_t)((uint64_t)((uint64_t)i64Result >> THERMOCOUPLE_24BIT_SHIFTING));
        }
        return (i64Result);
    }        
#endif /* End (!TC_CALC_FLOAT) */


/*******************************************************************************
* Function Name: Thermocouple_GetTemperature
********************************************************************************
*
* Summary:
*  This function takes thermocouple voltage in microvolt as ui8Idx/p and calculates
*  temperature corresponding to the thermocouple voltage. The order of the
*  polynomial and the polynomial coefficients used to convert voltage to
*  temperature are dependent on the type of thermocouple selected.
*
* Parameters:
*  int32_t i32Voltage : Thermocouple i32Voltage measured in microvolts.
*
* Return:
*  int32_t : Temperature in 1/100 degree C
*
*******************************************************************************/
int32_t Thermocouple_GetTemperature(int32_t i32Voltage, teTHERMOCOUPLE eTyp) 
{
    /***************************************
    *  Customizer Generated Coefficients
    ***************************************/
    const uint8_t ui8Thermocouple_coeffVT_gap = 11;  
    const int32_t aai32Thermocouple_voltRange[THERMOCOUPLE_CNT][3] = 
    {
        {291, 2431, 0},       // B
        {0,0,0},              // C
        {0,0,0},              // D
        {-8825, 0, 0},        // E
        {0, 42919, 0},        // J
        {0, 20644, 0},        // K
        {0,0,0},              // L
        {-3990, 0, 20613},    // N
        {1923, 11361, 19739}, // R
        {1874, 10332, 17536}, // S
        {-5603, 0, 0},        // T
        {0,0,0}               // U
    };

    const int8_t ai8Thermocouple_order_vt[THERMOCOUPLE_CNT] = 
    {
        0x09u,        // B 
        0,            // C
        0,            // D
        0x0Au,        // E
        0x08u,        // J
        0x0Au,        // K
        0,            // L
        0x0Au,        // N
        0x08u,        // R
        0x0Au,        // S
        0x08u,        // T
        0             // U
    };    

    const int8_t  ai8Thermocouple_vt_range_len[THERMOCOUPLE_CNT] = 
    {
        0x02u,        // B 
        0x02u,        // C
        0x02u,        // D
        0x02u,        // E
        0x02u,        // J
        0x02u,        // K
        0x02u,        // L
        0x03u,        // N
        0x03u,        // R
        0x03u,        // S
        0x02u,        // T
        0x02u         // U 
    };

    uint8_t ui8Thermocouple_coeffVT_StartIdx =  eTyp * ui8Thermocouple_coeffVT_gap;
    
#if(TC_CALC_FLOAT)

    /* Variable to store temperature */
    float   fTemp=0.0f;
    float   fVoltageNorm=0.0f;
    uint8_t ui8Idx=0u;
    

    if (ai8Thermocouple_vt_range_len[eTyp] == Thermocouple_THREE)
    {
        fVoltageNorm = (float)i32Voltage;

        if(i32Voltage < aai32Thermocouple_voltRange[eTyp][Thermocouple_RANGE_MAS_0] )
        {
            for (ui8Idx=ai8Thermocouple_order_vt[eTyp]+ui8Thermocouple_coeffVT_StartIdx - 1u; ui8Idx > ui8Thermocouple_coeffVT_StartIdx; ui8Idx--)
            {
                fTemp = (fTemp + aafThermocouple_coeffVT[ui8Idx+ui8Thermocouple_coeffVT_StartIdx][Thermocouple_RANGE_MAS_0]) * fVoltageNorm;
            }
            fTemp = (fTemp + aafThermocouple_coeffVT[Thermocouple_FIRST_EL_MAS+ui8Thermocouple_coeffVT_StartIdx][Thermocouple_RANGE_MAS_0]);
        }

        else if(i32Voltage <= aai32Thermocouple_voltRange[eTyp][Thermocouple_RANGE_MAS_1] )
        {
            for (ui8Idx=ai8Thermocouple_order_vt[eTyp] - 1u; ui8Idx > 0u; ui8Idx--)
            {
                fTemp = (fTemp + aafThermocouple_coeffVT[ui8Idx+ui8Thermocouple_coeffVT_StartIdx][Thermocouple_RANGE_MAS_1]) * fVoltageNorm;
            }
            fTemp = (fTemp + aafThermocouple_coeffVT[Thermocouple_FIRST_EL_MAS+ui8Thermocouple_coeffVT_StartIdx][Thermocouple_RANGE_MAS_1]);
        }

        else if(i32Voltage <= aai32Thermocouple_voltRange[eTyp][Thermocouple_RANGE_MAS_2] )
        {
            for (ui8Idx=ai8Thermocouple_order_vt[eTyp] - 1u; ui8Idx > 0u; ui8Idx--)
            {
                fTemp = (fTemp + aafThermocouple_coeffVT[ui8Idx+ui8Thermocouple_coeffVT_StartIdx][Thermocouple_RANGE_MAS_2]) * fVoltageNorm;
            }
            fTemp = (fTemp + aafThermocouple_coeffVT[Thermocouple_FIRST_EL_MAS+ui8Thermocouple_coeffVT_StartIdx][Thermocouple_RANGE_MAS_2]);
        }

        else
        {
            for (ui8Idx=ai8Thermocouple_order_vt[eTyp] - 1u; ui8Idx > 0u; ui8Idx--)
            {
                fTemp = (fTemp + aafThermocouple_coeffVT[ui8Idx+ui8Thermocouple_coeffVT_StartIdx][Thermocouple_RANGE_MAS_3]) * fVoltageNorm;
            }
            fTemp = (fTemp + aafThermocouple_coeffVT[Thermocouple_FIRST_EL_MAS+ui8Thermocouple_coeffVT_StartIdx][Thermocouple_RANGE_MAS_3]);
        }
    }
    else
    {
        fVoltageNorm = (float)i32Voltage;

        if(i32Voltage < aai32Thermocouple_voltRange[eTyp][Thermocouple_RANGE_MAS_0] )
        {
            for (ui8Idx=ai8Thermocouple_order_vt[eTyp] - 1u; ui8Idx > 0u; ui8Idx--)
            {
                fTemp = (fTemp + aafThermocouple_coeffVT[ui8Idx+ui8Thermocouple_coeffVT_StartIdx][Thermocouple_RANGE_MAS_0]) * fVoltageNorm;
            }
            fTemp = (fTemp + aafThermocouple_coeffVT[Thermocouple_FIRST_EL_MAS+ui8Thermocouple_coeffVT_StartIdx][Thermocouple_RANGE_MAS_0]);
        }

        else if(i32Voltage <= aai32Thermocouple_voltRange[eTyp][Thermocouple_RANGE_MAS_1] )
        {
            for (ui8Idx=ai8Thermocouple_order_vt[eTyp] - 1u; ui8Idx > 0u; ui8Idx--)
            {
                fTemp = (fTemp + aafThermocouple_coeffVT[ui8Idx+ui8Thermocouple_coeffVT_StartIdx][Thermocouple_RANGE_MAS_1]) * fVoltageNorm;
            }
            fTemp = (fTemp + aafThermocouple_coeffVT[Thermocouple_FIRST_EL_MAS+ui8Thermocouple_coeffVT_StartIdx][Thermocouple_RANGE_MAS_1]);
        }

        else
        {
            for (ui8Idx=ai8Thermocouple_order_vt[eTyp] - 1u; ui8Idx > 0u; ui8Idx--)
            {
                fTemp = (fTemp + aafThermocouple_coeffVT[ui8Idx][Thermocouple_RANGE_MAS_2+ui8Thermocouple_coeffVT_StartIdx]) * fVoltageNorm;
            }
            fTemp = (fTemp + aafThermocouple_coeffVT[Thermocouple_FIRST_EL_MAS+ui8Thermocouple_coeffVT_StartIdx][Thermocouple_RANGE_MAS_2]);
        }
    }

    /* End  ai8Thermocouple_vt_range_len[eTyp] == Thermocouple_THREE */

    return ((int32_t)(fTemp));
}
#else

    const int8_t aai8Thermocouple_xScaleVT[THERMOCOUPLE_CNT][4] = 
    {
        {10, 11, 14, 0},        // B 
        {0,0,0,0},              // C
        {0,0,0,0},              // D
        {12, 14, 17, 0},        // E
        {13, 16, 16, 0},        // J
        {13, 15, 16, 0},        // K
        {0,0,0,0},              // L
        {11, 12, 15, 15},       // N
        {11, 14, 14, 12},       // R
        {11, 15, 14, 12},       // S
        {11, 13, 15, 0},        // T
        {0,0,0,0}               // U
    };

    const int8_t aai8Thermocouple_coefScaleVT[THERMOCOUPLE_CNT][4] = 
    {
        {3, 7, 4, 0},        // B 
        {0,0,0,0},           // C
        {0,0,0,0},           // D
        {5, 5, 4, 0},        // E
        {10, 9, 4, 0},       // J
        {7, 1, 7, 0},        // K
        {0,0,0,0},           // L
        {2, 7, 9, 11},       // N
        {6, 5, 10, 6},       // R
        {8, 1, 10, 8},       // S
        {3, 9, 10, 0},       // T
        {0,0,0,0}            // U
    };

    /* Variable to store temperature */
    int32_t i32Temp=0;
    uint8_t ui8Idx=0u;

    if (ai8Thermocouple_vt_range_len[eTyp] == THERMOCOUPLE_THREE)
    {
        if(i32Voltage < aai32Thermocouple_voltRange[eTyp][THERMOCOUPLE_RANGE_MAS_0] )
        {
            if (i32Voltage < 0)
            {
                i32Voltage = -i32Voltage;
                i32Voltage = (int32_t)((uint64_t)((uint64_t)i32Voltage << 
                                               (THERMOCOUPLE_IN_NORMALIZATION_VT - 
                                                aai8Thermocouple_xScaleVT[eTyp][THERMOCOUPLE_RANGE_MAS_0])));
                i32Voltage = -i32Voltage;
            }
            else
            {
                i32Voltage = (int32_t)((uint64_t)((uint64_t)i32Voltage << 
                                               (THERMOCOUPLE_IN_NORMALIZATION_VT - 
                                                aai8Thermocouple_xScaleVT[eTyp][THERMOCOUPLE_RANGE_MAS_0])));
            }            

            for (ui8Idx=ai8Thermocouple_order_vt[eTyp] - 1u; ui8Idx > 0u; ui8Idx--)
            {
                i32Temp = Thermocouple_MultShift24((aai32Thermocouple_coeffVT[ui8Idx+ui8Thermocouple_coeffVT_StartIdx][THERMOCOUPLE_RANGE_MAS_0] +
                       i32Temp), i32Voltage);
            }

            if(aai8Thermocouple_coefScaleVT[eTyp][THERMOCOUPLE_RANGE_MAS_0] < 0)
            {
                i32Temp = (int32_t)((uint64_t)((uint64_t)(int32_t)(i32Temp + 
                        aai32Thermocouple_coeffVT[THERMOCOUPLE_FIRST_EL_MAS+ui8Thermocouple_coeffVT_StartIdx][THERMOCOUPLE_RANGE_MAS_0]) << 
                        aai8Thermocouple_coefScaleVT[eTyp][THERMOCOUPLE_RANGE_MAS_0]));
            }

            else
            {
                i32Temp = (int32_t)((uint32_t)((uint64_t)(int32_t)(i32Temp + 
                        aai32Thermocouple_coeffVT[THERMOCOUPLE_FIRST_EL_MAS+ui8Thermocouple_coeffVT_StartIdx][THERMOCOUPLE_RANGE_MAS_0]) >> 
                        aai8Thermocouple_coefScaleVT[eTyp][THERMOCOUPLE_RANGE_MAS_0]));
            }
        }

        else if(i32Voltage <= aai32Thermocouple_voltRange[eTyp][THERMOCOUPLE_RANGE_MAS_1] )
        {
            if (i32Voltage < 0)
            {
                i32Voltage = -i32Voltage;
                i32Voltage = (int32_t)((uint64_t)((uint64_t)i32Voltage << 
                                               (THERMOCOUPLE_IN_NORMALIZATION_VT - 
                                                aai8Thermocouple_xScaleVT[eTyp][THERMOCOUPLE_RANGE_MAS_1])));
                i32Voltage = -i32Voltage;
            }
            else
            {
                i32Voltage = (int32_t)((uint64_t)((uint64_t)i32Voltage << 
                                               (THERMOCOUPLE_IN_NORMALIZATION_VT - 
                                                aai8Thermocouple_xScaleVT[eTyp][THERMOCOUPLE_RANGE_MAS_1])));
            }              
            
            for (ui8Idx=ai8Thermocouple_order_vt[eTyp] - 1u; ui8Idx > 0u; ui8Idx--)
            {
                i32Temp = Thermocouple_MultShift24((aai32Thermocouple_coeffVT[ui8Idx+ui8Thermocouple_coeffVT_StartIdx][THERMOCOUPLE_RANGE_MAS_1] +
                       i32Temp), i32Voltage);
            }

            if(aai8Thermocouple_coefScaleVT[eTyp][THERMOCOUPLE_RANGE_MAS_1] < 0)
            {
                i32Temp = (int32_t)((uint64_t)((uint64_t)(int32_t)(i32Temp + 
                        aai32Thermocouple_coeffVT[THERMOCOUPLE_FIRST_EL_MAS+ui8Thermocouple_coeffVT_StartIdx][THERMOCOUPLE_RANGE_MAS_1]) << 
                        aai8Thermocouple_coefScaleVT[eTyp][THERMOCOUPLE_RANGE_MAS_1]));
            }

            else
            {
                i32Temp = (int32_t)((uint32_t)((uint64_t)(int32_t)(i32Temp + 
                        aai32Thermocouple_coeffVT[THERMOCOUPLE_FIRST_EL_MAS+ui8Thermocouple_coeffVT_StartIdx][THERMOCOUPLE_RANGE_MAS_1]) >> 
                        aai8Thermocouple_coefScaleVT[eTyp][THERMOCOUPLE_RANGE_MAS_1]));
            }

        }

        else if(i32Voltage <= aai32Thermocouple_voltRange[eTyp][THERMOCOUPLE_RANGE_MAS_2] )
        {
            if (i32Voltage < 0)
            {
                i32Voltage = -i32Voltage;
                i32Voltage = (int32_t)((uint64_t)((uint64_t)i32Voltage << 
                                               (THERMOCOUPLE_IN_NORMALIZATION_VT - 
                                                aai8Thermocouple_xScaleVT[eTyp][THERMOCOUPLE_RANGE_MAS_2])));
                i32Voltage = -i32Voltage;
            }
            else
            {
                i32Voltage = (int32_t)((uint64_t)((uint64_t)i32Voltage << 
                                               (THERMOCOUPLE_IN_NORMALIZATION_VT - 
                                                aai8Thermocouple_xScaleVT[eTyp][THERMOCOUPLE_RANGE_MAS_2])));
            }              

            for (ui8Idx=ai8Thermocouple_order_vt[eTyp] - 1u; ui8Idx > 0u; ui8Idx--)
            {
                i32Temp = Thermocouple_MultShift24((aai32Thermocouple_coeffVT[ui8Idx+ui8Thermocouple_coeffVT_StartIdx][THERMOCOUPLE_RANGE_MAS_2] +
                       i32Temp), i32Voltage);
            }

            if(aai8Thermocouple_coefScaleVT[eTyp][THERMOCOUPLE_RANGE_MAS_2] < 0)
            {
                i32Temp = (int32_t)((uint64_t)((uint64_t)(int32_t)(i32Temp + 
                        aai32Thermocouple_coeffVT[THERMOCOUPLE_FIRST_EL_MAS+ui8Thermocouple_coeffVT_StartIdx][THERMOCOUPLE_RANGE_MAS_2]) << 
                        aai8Thermocouple_coefScaleVT[eTyp][THERMOCOUPLE_RANGE_MAS_2]));
            }

            else
            {
                i32Temp = (int32_t)((uint32_t)((uint64_t)(int32_t)(i32Temp + 
                        aai32Thermocouple_coeffVT[THERMOCOUPLE_FIRST_EL_MAS+ui8Thermocouple_coeffVT_StartIdx][THERMOCOUPLE_RANGE_MAS_2]) >> 
                        aai8Thermocouple_coefScaleVT[eTyp][THERMOCOUPLE_RANGE_MAS_2]));
            }
        }

        else
        {
            if (i32Voltage < 0)
            {
                i32Voltage = -i32Voltage;
                i32Voltage = (int32_t)((uint64_t)((uint64_t)i32Voltage << 
                                               (THERMOCOUPLE_IN_NORMALIZATION_VT - 
                                                aai8Thermocouple_xScaleVT[eTyp][THERMOCOUPLE_RANGE_MAS_3])));
                i32Voltage = -i32Voltage;
            }
            else
            {
                i32Voltage = (int32_t)((uint64_t)((uint64_t)i32Voltage << 
                                               (THERMOCOUPLE_IN_NORMALIZATION_VT - 
                                                aai8Thermocouple_xScaleVT[eTyp][THERMOCOUPLE_RANGE_MAS_3])));
            }              

            for (ui8Idx=ai8Thermocouple_order_vt[eTyp] - 1u; ui8Idx > 0u; ui8Idx--)
            {
                i32Temp = Thermocouple_MultShift24((aai32Thermocouple_coeffVT[ui8Idx+ui8Thermocouple_coeffVT_StartIdx][THERMOCOUPLE_RANGE_MAS_3] +
                       i32Temp), i32Voltage);
            }

            if(aai8Thermocouple_coefScaleVT[eTyp][THERMOCOUPLE_RANGE_MAS_3] < 0)
            {
                i32Temp = (int32_t)((uint64_t)((uint64_t)(int32_t)(i32Temp + 
                        aai32Thermocouple_coeffVT[THERMOCOUPLE_FIRST_EL_MAS+ui8Thermocouple_coeffVT_StartIdx][THERMOCOUPLE_RANGE_MAS_3]) << 
                        aai8Thermocouple_coefScaleVT[eTyp][THERMOCOUPLE_RANGE_MAS_3]));
            }

            else
            {
                i32Temp = (int32_t)((uint32_t)((uint64_t)(int32_t)(i32Temp + 
                        aai32Thermocouple_coeffVT[THERMOCOUPLE_FIRST_EL_MAS+ui8Thermocouple_coeffVT_StartIdx][THERMOCOUPLE_RANGE_MAS_3]) >> 
                        aai8Thermocouple_coefScaleVT[eTyp][THERMOCOUPLE_RANGE_MAS_3]));
            }
        }
    }
    else
    {
        if(i32Voltage < aai32Thermocouple_voltRange[eTyp][THERMOCOUPLE_RANGE_MAS_0] )
        {
            if (i32Voltage < 0)
            {
                i32Voltage = -i32Voltage;
                i32Voltage = (int32_t)((uint64_t)((uint64_t)i32Voltage << 
                                               (THERMOCOUPLE_IN_NORMALIZATION_VT - 
                                                aai8Thermocouple_xScaleVT[eTyp][THERMOCOUPLE_RANGE_MAS_0])));
                i32Voltage = -i32Voltage;
            }
            else
            {
                i32Voltage = (int32_t)((uint64_t)((uint64_t)i32Voltage << 
                                               (THERMOCOUPLE_IN_NORMALIZATION_VT - 
                                                aai8Thermocouple_xScaleVT[eTyp][THERMOCOUPLE_RANGE_MAS_0])));
            }            

            for (ui8Idx=ai8Thermocouple_order_vt[eTyp] - 1u; ui8Idx > 0u; ui8Idx--)
            {
                i32Temp = Thermocouple_MultShift24((aai32Thermocouple_coeffVT[ui8Idx+ui8Thermocouple_coeffVT_StartIdx][THERMOCOUPLE_RANGE_MAS_0] +
                       i32Temp), i32Voltage);
            }

            if(aai8Thermocouple_coefScaleVT[eTyp][THERMOCOUPLE_RANGE_MAS_0] < 0)
            {
                i32Temp = (int32_t)((uint64_t)((uint64_t)(int32_t)(i32Temp + 
                        aai32Thermocouple_coeffVT[THERMOCOUPLE_FIRST_EL_MAS+ui8Thermocouple_coeffVT_StartIdx][THERMOCOUPLE_RANGE_MAS_0]) << 
                        aai8Thermocouple_coefScaleVT[eTyp][THERMOCOUPLE_RANGE_MAS_0]));
            }

            else
            {
                i32Temp = (int32_t)((uint32_t)((uint64_t)(int32_t)(i32Temp + 
                        aai32Thermocouple_coeffVT[THERMOCOUPLE_FIRST_EL_MAS+ui8Thermocouple_coeffVT_StartIdx][THERMOCOUPLE_RANGE_MAS_0]) >> 
                        aai8Thermocouple_coefScaleVT[eTyp][THERMOCOUPLE_RANGE_MAS_0]));
            }
        }

        else if(i32Voltage <= aai32Thermocouple_voltRange[eTyp][THERMOCOUPLE_RANGE_MAS_1] )
        {
            if (i32Voltage < 0)
            {
                i32Voltage = -i32Voltage;
                i32Voltage = (int32_t)((uint64_t)((uint64_t)i32Voltage << 
                                               (THERMOCOUPLE_IN_NORMALIZATION_VT - 
                                                aai8Thermocouple_xScaleVT[eTyp][THERMOCOUPLE_RANGE_MAS_1])));
                i32Voltage = -i32Voltage;
            }
            else
            {
                i32Voltage = (int32_t)((uint64_t)((uint64_t)i32Voltage << 
                                               (THERMOCOUPLE_IN_NORMALIZATION_VT - 
                                                aai8Thermocouple_xScaleVT[eTyp][THERMOCOUPLE_RANGE_MAS_1])));
            }              
            
            for (ui8Idx=ai8Thermocouple_order_vt[eTyp] - 1u; ui8Idx > 0u; ui8Idx--)
            {
                i32Temp = Thermocouple_MultShift24((aai32Thermocouple_coeffVT[ui8Idx+ui8Thermocouple_coeffVT_StartIdx][THERMOCOUPLE_RANGE_MAS_1] +
                       i32Temp), i32Voltage);
            }

            if(aai8Thermocouple_coefScaleVT[eTyp][THERMOCOUPLE_RANGE_MAS_1] < 0)
            {
                i32Temp = (int32_t)((uint64_t)((uint64_t)(int32_t)(i32Temp + 
                        aai32Thermocouple_coeffVT[THERMOCOUPLE_FIRST_EL_MAS+ui8Thermocouple_coeffVT_StartIdx][THERMOCOUPLE_RANGE_MAS_1]) << 
                        aai8Thermocouple_coefScaleVT[eTyp][THERMOCOUPLE_RANGE_MAS_1]));
            }

            else
            {
                i32Temp = (int32_t)((uint32_t)((uint64_t)(int32_t)(i32Temp + 
                        aai32Thermocouple_coeffVT[THERMOCOUPLE_FIRST_EL_MAS+ui8Thermocouple_coeffVT_StartIdx][THERMOCOUPLE_RANGE_MAS_1]) >> 
                        aai8Thermocouple_coefScaleVT[eTyp][THERMOCOUPLE_RANGE_MAS_1]));
            }
        }

        else
        {
            if (i32Voltage < 0)
            {
                i32Voltage = -i32Voltage;
                i32Voltage = (int32_t)((uint64_t)((uint64_t)i32Voltage << 
                                               (THERMOCOUPLE_IN_NORMALIZATION_VT - 
                                                aai8Thermocouple_xScaleVT[eTyp][THERMOCOUPLE_RANGE_MAS_2])));
                i32Voltage = -i32Voltage;
            }
            else
            {
                i32Voltage = (int32_t)((uint64_t)((uint64_t)i32Voltage << 
                                               (THERMOCOUPLE_IN_NORMALIZATION_VT - 
                                                aai8Thermocouple_xScaleVT[eTyp][THERMOCOUPLE_RANGE_MAS_2])));
            }              

            for (ui8Idx=ai8Thermocouple_order_vt[eTyp] - 1u; ui8Idx > 0u; ui8Idx--)
            {
                i32Temp = Thermocouple_MultShift24((aai32Thermocouple_coeffVT[ui8Idx+ui8Thermocouple_coeffVT_StartIdx][THERMOCOUPLE_RANGE_MAS_2] +
                       i32Temp), i32Voltage);
            }

            if(aai8Thermocouple_coefScaleVT[eTyp][THERMOCOUPLE_RANGE_MAS_2] < 0)
            {
                i32Temp = (int32_t)((uint64_t)((uint64_t)(int32_t)(i32Temp + 
                        aai32Thermocouple_coeffVT[THERMOCOUPLE_FIRST_EL_MAS+ui8Thermocouple_coeffVT_StartIdx][THERMOCOUPLE_RANGE_MAS_2]) << 
                        aai8Thermocouple_coefScaleVT[eTyp][THERMOCOUPLE_RANGE_MAS_2]));
            }

            else
            {
                i32Temp = (int32_t)((uint32_t)((uint64_t)(int32_t)(i32Temp + 
                        aai32Thermocouple_coeffVT[THERMOCOUPLE_FIRST_EL_MAS+ui8Thermocouple_coeffVT_StartIdx][THERMOCOUPLE_RANGE_MAS_2]) >> 
                        aai8Thermocouple_coefScaleVT[eTyp][THERMOCOUPLE_RANGE_MAS_2]));
            }
        }
    }

    /* End ai8Thermocouple_vt_range_len[eTyp] == Thermocouple_THREE */

    return (i32Temp);
}
#endif /* End TC_CALC_FLOAT */


/*******************************************************************************
* Function Name: Thermocouple_GetVoltage
********************************************************************************
*
* Summary:
*  This function takes the temperature as input and provides the expected
*  i32Voltage for that temperature.
*
* Parameters:
*  int32_t temperature : Temperature of the cold junction in 1/100 degree C
*
* Return:
*  int32_t : Expected i32Voltage output of the thermocouple in microvolts
*
*******************************************************************************/
int32_t Thermocouple_GetVoltage(int32_t temperature, teTHERMOCOUPLE eTyp) 
{
/***************************************
*  Customizer Generated Coefficients
***************************************/                                                                                   
    const int8_t Thermocouple_order_tv[THERMOCOUPLE_CNT] = 
    {
        0x07u,        // B 
        0,            // C
        0,            // D
        0x09u,        // E
        0x08u,        // J
        0x09u,        // K
        0,            // L
        0x09u,        // N
        0x08u,        // R
        0x08u,        // S
        0x09u,        // T
        0             // U     
    };

#if (TC_CALC_FLOAT)
                                                                
    const float Thermocouple_coeffTV[THERMOCOUPLE_CNT][9]={
        //            0,             1,             2,             3,             4,             5,              6,             7,             8     Type
        {             0,  -0.002465082,  5.904042E-07, -1.325793E-12,  1.566829E-17, -1.694453E-22,   6.299035E-28,             0,             0}, // B
        {             0,             0,             0,             0,             0,             0,              0,             0,             0}, // C
        {             0,             0,             0,             0,             0,             0,              0,             0,             0}, // D   
        {   -0.01105851,     0.5864896,  4.721644E-06, -1.207539E-10,    5.6174E-14, -1.395674E-17,   1.897255E-21, -1.344229E-25,  3.834267E-30}, // E 
        {   -0.0335813,      0.5038356,   3.10796E-06,  -9.33262E-11,  -1.12089E-14,    4.1809E-18,   -4.76112E-22,   1.83557E-26,             0}, // J
        {    0.0146116,      0.3945689,  2.426433E-06, -1.222199E-10,  2.953216E-14, -8.253639E-18,    1.09979E-21, -7.373136E-26,  1.975709E-30}, // K
        {            0,              0,             0,             0,             0,             0,              0,             0,             0}, // L                                       
        {     -0.33751,      0.2604841,   6.37101E-07,   2.01537E-10,  6.861415E-14, -3.331119E-17,   5.412231E-21, -3.969246E-25,   1.10614E-29}, // N
        {    0.0164821,     0.05271165,   1.51603E-06,   1.08466E-11,  -3.22759E-14,   7.03863E-18,   -6.22194E-22,   1.98772E-26,             0}, // R
        {   -0.0326425,     0.05402764,   1.31413E-06,  -2.29112E-11,  -8.67203E-15,   2.31613E-18,   -2.26983E-22,   7.94312E-27,             0}, // S
        {    0.1654603,      0.3871125,   3.74869E-06, -6.707544E-11,  7.296784E-14, -1.601324E-17,   1.536673E-21, -6.496373E-26,  8.418897E-31}, // T
        {            0,              0,             0,             0,             0,             0,              0,             0,             0}  // U  
    };   

    /* Variable to store calculated voltage */
    uint8_t ui8Idx=0u;
    float f32Voltage=0.0f;
    float temperatureNorm=0.0f;

    temperatureNorm  = (float)temperature;

    for (ui8Idx = Thermocouple_order_tv[eTyp] - 1u; ui8Idx > 0u; ui8Idx--)
    {
        f32Voltage = (Thermocouple_coeffTV[eTyp][ui8Idx] + f32Voltage) * temperatureNorm;
    }
    f32Voltage = (f32Voltage + Thermocouple_coeffTV[eTyp][Thermocouple_FIRST_EL_MAS]);

    return ((int32_t) (f32Voltage));
}
#else                                  
    const int32_t Thermocouple_coeffTV[THERMOCOUPLE_CNT][9] = {
        //        0,         1,         2,         3,         4,         5,          6,         7,         8      Type   
         {        0,     -2585,     20286,     -1493,       578,      -205,         25,         0,         0}, // B
         {        0,         0,         0,         0,         0,         0,          0,         0,         0}, // C
         {        0,         0,         0,         0,         0,         0,          0,         0,         0}, // D     
         {        0,    614979,    162234,   -135957,   2072455, -16872665,   75158001,-174490880, 163091688}, // E
         {       -1,    528310,    106789,   -105076,   -413535,   5054398,  -18860739,  23827061,         0}, // J
         {        0,    413735,     83372,   -137607,   1089544,  -9978037,   43567176, -95708775,  84037362}, // K
         {        0,         0,         0,         0,         0,         0,          0,         0,         0}, // L
         {      -11,    273137,     21891,    226910,   2531415, -40270757,  214400579,-515237574, 470499927}, // N
         {        1,     55272,     52090,     12212,  -1190771,   8509182,  -24647644,  25802081,         0}, // R
         {       -1,     56652,     45153,    -25796,   -319941,   2800029,   -8991723,  10310759,         0}, // S
         {        5,    405917,    128804,    -75520,   2692038, -19358823,   60873887, -84327741,  35810029}, // T
         {        0,         0,         0,         0,         0,         0,          0,         0,         0}  // U 
     };   

    /* Variable to store calculated voltage */
    uint8_t ui8Idx=0u;
    int32_t i32Voltage=0;

    if (temperature < 0)
    {
        temperature = -temperature;
        temperature = (int32_t)((uint64_t)((uint64_t)temperature << 
                                       (THERMOCOUPLE_IN_NORMALIZATION_TV - THERMOCOUPLE_X_SCALE_TV)));
        temperature = -temperature;
    }
    else
    {
        temperature = (int32_t)((uint64_t)((uint64_t)temperature << 
                                       (THERMOCOUPLE_IN_NORMALIZATION_TV - THERMOCOUPLE_X_SCALE_TV)));
    }
    
    for (ui8Idx = Thermocouple_order_tv[eTyp] - 1u; ui8Idx > 0u; ui8Idx--)
    {
        i32Voltage = Thermocouple_MultShift24((Thermocouple_coeffTV[eTyp][ui8Idx] + i32Voltage), temperature);
    }

    i32Voltage = (int32_t)((uint32_t)((uint64_t)(int32_t)(i32Voltage + Thermocouple_coeffTV[eTyp][THERMOCOUPLE_FIRST_EL_MAS]) >> 
                                        THERMOCOUPLE_COEF_SCALE_TV));

    return (i32Voltage);
}
#endif /* End TC_CALC_FLOAT */

/* [] END OF FILE */
