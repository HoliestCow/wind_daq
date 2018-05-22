namespace cpp wind
namespace java org.wind
namespace csharp org.wind

include "Spectrum.thrift"

struct DoseInfo
{
  /* Estimated Gamma dose at detector in Sv/hr */ 
  1: double gammaDose;

  /* Estimated Neutron dose at detector in Sv/hr */ 
  2: double neutronDose;
}

struct ShieldingInfo
{
  1: double effectiveZ;
  2: double effectiveAD;
}

enum ActivityUnit
{
  BQ = 1,
  KG = 2
}

struct ActivityInfo
{
  1: double activity;
  2: ActivityUnit activityUnit
}

/***********************************************************************/

enum Classification
{
  /* Sample consistent with typical background */
  BACKGROUND          = 100, 

  /* Excess of NORM in equalibrium (K40, Ra226, Th232) */
  NORM                = 101,

  /* Natural nuclides in concentrates (La138, Ac228), i.e., technically-enhanced */
  TENORM              = 102,

  /* Medical nuclides associated used introvenously (Diagnostic, Theraputic) */
  MEDICAL             = 200,   

  /* Parents of medical nuclides associated with shipments */
  MEDICAL_SHIPMENT    = 201,  

  /* Medium/Long lived associated with gauges, radiography, and 
     medical devices */
  INDUSTRIAL = 300, 

  /* Industrial sources in large quantities with heavy shielding */
  RDD        = 301,

  /* SNM, DU, and other materials controled per DOE table */
  NUCLEAR    = 400,

  /* Nuclides not associated with intential usage */
  OTHER      = 500,

  /* Short lived activation produces associated with neutron imaging
     and sterialization */
  ACTIVATION = 501,   

  /* Short lived products associated with isotopic production */
  WASTE      = 502,   

  /* Short lived products associated with fission (fallout, ie. Cs134) */
  FISSION    = 503   
}

struct ClassificationSummaryEntry
{
  1: Classification classification;
  2: double probability;
}

/* FIXME - the category table is a mixture of subtypes 
  ie, (Waste, Activation)==Other  and similar uses Medical!=Medical_shipment.
  Ideally, we should be consistent.  A better categorization should be 
  discussed.
*/



/* Notes:
     Effective distance:
       when computing a spectrum accumulated at different distances for 
      different lengths of time, the effective distance is required to 
      convert from counts at the detector to counts at the source.
      The effective distance is given by:
          R_eff = sqrt( (\sum_i T_i)/ (sum_j T_j/R_j^2))

    Effective atomic number:
      When computing the effect of shielding between a source and the 
      detector, the effective stopping power must be weighted by the atomic
      number of the materials. For example, water (average Z=3.3) has a
      stopping power more associate with oxygen than the hydrogen.  
      The effective Z is computed as 
         Z_eff = (sum_i F_i*Z_i^2.94 )^(1/2.94)


      see: https://en.wikipedia.org/wiki/Effective_atomic_number

   Effective areal density:
     The effect of multiple shielding material on radiation is to add to the
     total shielding.  Thus the effective areal density is the sum
     of areal density of each of the shielding materials.
*/


/************************************************************************************/

/**
 * ZA Numbered isotopes.  
 *   ZA number is 10000 * Z + A + 300 * Metastable + 1000 * Process
 * Z: Number of protons
 * A: Atomic mass (protons + neutrons) (natural/unspecified = 0)
 * Process: 1=XRay, 2=N_G (capture), 3=A_N, 4=A_P, 5=N_N (inelastic)
 * 
 * Derived from SIGMA list
 */
enum Nuclide {
  Unknown = 0,
  Incoherent = 1,
  HEU = 2,
  LEU = 3,
  WGPU = 4,
  RGPU = 5,
  NEUTRONS = 6,

  H_N_G   =  12000,
  H_3     =  10003,
  LI_N_G  =  32000,
  LI_A_G  =  33000,
  BE_N_G  =  42000,
  BE_A_N  =  43000,
  BE_7    =  40007,
  B_A_G   =  53000,
  B_N_G   =  52000,
  C_A_G   =  63000,
  C_N_G   =  62000,
  C_11    =  60011,
  C_14    =  60014,
  N_A_P   =  74000,
  N_N_G   =  72000,
  N_13    =  70013,
  N_16    =  70016,
  O_A_N   =  83000,
  O_15    =  80015,
  F_A_G   =  93000,
  F_N_G   =  92000,
  F_18    =  90018,
  NA_A_G  = 113000,
  NA_22   = 110022,
  NA_24   = 110024,
  NA_25   = 110025,
  MG_A_G  = 123000,
  MG_27   = 120027,
  MG_28   = 120028,
  AL_N_G  = 121000,
  AL_A_P  = 124000,
  AL_26   = 130026,
  AL_28   = 130028,
  SI_N_G  = 142000,
  SI_31   = 140031,
  P_32    = 150032,
  P_33    = 150033,
  CL_A_P  = 174000,
  CL_N_G  = 172000,
  CL_36   = 170036,
  CL_38   = 170038,
  AR_41   = 180041,
  K_A_P   = 194000,
  K_40    = 190040,
  K_42    = 190042,
  K_43    = 190043,
  CA_45   = 200045,
  CA_47   = 200047,
  SC_44   = 210044,
  SC_46   = 210046,
  SC_47   = 210047,
  SC_48   = 210048,
  SC_44M  = 210344,
  TI_44   = 220044,
  V_48    = 230048,
  CR_N_G  = 242000,
  CR_48   = 240048,
  CR_51   = 240051,
  MN_51   = 250051,
  MN_52   = 250052,
  MN_54   = 250054,
  MN_56   = 250056,
  MN_52M  = 250352,
  FE_N_G  = 252000,
  FE_52   = 260052,
  FE_55   = 260055,
  FE_59   = 260059,
  CO_55   = 270055,
  CO_56   = 270056,
  CO_57   = 270057,
  CO_58   = 270058,
  CO_60   = 270060,
  CO_61   = 270061,
  CO_62   = 270062,
  NI_N_G  = 282000,
  NI_56   = 280056,
  NI_57   = 280057,
  NI_63   = 280063,
  CU_N_G  = 292000,
  CU_61   = 290061,
  CU_62   = 290062,
  CU_64   = 290064,
  CU_67   = 290067,
  ZN_62   = 300062,
  ZN_65   = 300065,
  ZN_69   = 300069,
  ZN_69M  = 300369,
  GA_66   = 310066,
  GA_67   = 310067,
  GA_68   = 310068,
  GA_72   = 310072,
  GE_N_G  = 322000,
  GE_N_N  = 325000,
  GE_68   = 320068,
  GE_69   = 320069,
  GE_75   = 320075,
  GE_77   = 320077,
  GE_75M  = 320375,
  GE_77M  = 320377,
  AS_71   = 330071,
  AS_72   = 330072,
  AS_73   = 330073,
  AS_74   = 330074,
  AS_76   = 330076,
  AS_77   = 330077,
  SE_72   = 340072,
  SE_75   = 340075,
  BR_75   = 350075,
  BR_76   = 350076,
  BR_77   = 350077,
  BR_82   = 350082,
  BR_85   = 350085,
  KR_81   = 360081,
  KR_85   = 360085,
  KR_87   = 360087,
  KR_88   = 360088,
  KR_85M  = 360385,
  RB_81   = 370081,
  RB_82   = 370082,
  RB_83   = 370083,
  RB_84   = 370084,
  RB_86   = 370086,
  RB_88   = 370088,
  RB_89   = 370089,
  SR_82   = 380082,
  SR_83   = 380083,
  SR_85   = 380085,
  SR_89   = 380089,
  SR_90   = 380090,
  SR_91   = 380091,
  SR_92   = 380092,
  SR_87M  = 380387,
  Y_88    = 390088,
  Y_90    = 390090,
  Y_91    = 390091,
  Y_92    = 390092,
  Y_93    = 390093,
  ZR_88   = 400088,
  ZR_89   = 400089,
  ZR_95   = 400095,
  ZR_97   = 400097,
  NB_89   = 410089,
  NB_94   = 410094,
  NB_95   = 410095,
  NB_96   = 410096,
  NB_97   = 410097,
  NB_92M  = 410392,
  NB_95M  = 410395,
  MO_N_G  = 422000,
  MO_99   = 420099,
  MO_101  = 420101,
  TC_96   = 430096,
  TC_98   = 430098,
  TC_99   = 430099,
  TC_94M  = 430394,
  TC_95M  = 430395,
  TC_99M  = 430399,
  RU_97   = 440097,
  RU_103  = 440103,
  RU_105  = 440105,
  RU_106  = 440106,
  RH_99   = 450099,
  RH_100  = 450100,
  RH_101  = 450101,
  RH_102  = 450102,
  RH_105  = 450105,
  RH_106M = 450406,
  PD_102  = 460102,
  PD_109  = 460109,
  PD_109M = 460409,
  AGXRAYS = 471000,
  AG_108M = 470408,
  AG_110M = 470410,
  CDXRAYS = 481000,
  CD_N_G  = 482000,
  CD_109  = 480109,
  CD_115  = 480115,
  CD_115M = 480415,
  IN_N_G  = 492000,
  IN_111  = 490111,
  IN_114  = 490114,
  IN_116  = 490116,
  IN_113M = 490413,
  IN_114M = 490414,
  IN_115M = 490415,
  IN_116M = 490416,
  IN_116N = 490716,
  SN_N_G  = 502000,
  SN_113  = 500113,
  SN_125  = 500125,
  SN_117M = 500417,
  SB_122  = 510122,
  SB_124  = 510124,
  SB_125  = 510125,
  SB_126  = 510126,
  SB_127  = 510127,
  SB_129  = 510129,
  SB_120M = 510420,
  TE_121  = 520121,
  TE_129  = 520129,
  TE_132  = 520132,
  TE_133  = 520133,
  TE_119M = 520419,
  TE_123M = 520423,
  TE_129M = 520429,
  I_123   = 530123,
  I_124   = 530124,
  I_125   = 530125,
  I_126   = 530126,
  I_129   = 530129,
  I_131   = 530131,
  I_132   = 530132,
  I_133   = 530133,
  I_134   = 530134,
  I_135   = 530135,
  XE_122  = 540122,
  XE_125  = 540125,
  XE_127  = 540127,
  XE_133  = 540133,
  XE_135  = 540135,
  XE_138  = 540138,
  XE_129M = 540429,
  XE_131M = 540431,
  XE_133M = 540433,
  XE_135M = 540435,
  CS_130  = 550130,
  CS_131  = 550131,
  CS_132  = 550132,
  CS_134  = 550134,
  CS_136  = 550136,
  CS_137  = 550137,
  CS_138  = 550138,
  BA_128  = 560128,
  BA_131  = 560131,
  BA_133  = 560133,
  BA_139  = 560139,
  BA_140  = 560140,
  BA_141  = 560141,
  LA_N_G  = 572000,
  LA_140  = 570140,
  LA_141  = 570141,
  LA_142  = 570142,
  CE_139  = 580139,
  CE_141  = 580141,
  CE_143  = 580143,
  CE_144  = 580144,
  PR_144  = 590144,
  PR_145  = 590145,
  ND_147  = 600147,
  ND_149  = 600149,
  PM_143  = 610143,
  PM_145  = 610145,
  PM_146  = 610146,
  PM_147  = 610147,
  PM_149  = 610149,
  PM_151  = 610151,
  SM_145  = 620145,
  SM_151  = 620151,
  SM_153  = 620153,
  EU_146  = 630146,
  EU_150  = 630150,
  EU_152  = 630152,
  EU_154  = 630154,
  EU_155  = 630155,
  EU_156  = 630156,
  EU_152M = 630452,
  GD_146  = 640146,
  GD_153  = 640153,
  GD_159  = 640159,
  TB_149  = 650149,
  TB_160  = 650160,
  DY_165  = 660165,
  DY_166  = 660166,
  HO_166  = 670166,
  HO_167  = 670167,
  HO_166M = 670466,
  ER_169  = 680169,
  TM_167  = 690167,
  TM_170  = 690170,
  TM_171  = 690171,
  YB_167  = 700167,
  YB_169  = 700169,
  YB_175  = 700175,
  YB_169M = 700469,
  LUXRAYS = 711000,
  LU_169  = 710169,
  LU_171  = 710171,
  LU_172  = 710172,
  LU_173  = 710173,
  LU_177  = 710177,
  LU_171M = 710471,
  LU_177M = 710477,
  HF_171  = 720171,
  HF_172  = 720172,
  HF_173  = 720173,
  HF_175  = 720175,
  HF_181  = 720181,
  HF_178M = 720478,
  TA_178  = 730178,
  TA_182  = 730182,
  WXRAYS  = 741000,
  W_N_G   = 742000,
  W_178   = 740178,
  W_185   = 740185,
  W_187   = 740187,
  W_188   = 740188,
  RE_186  = 750186,
  RE_188  = 750188,
  OS_185  = 760185,
  OS_191  = 760191,
  OS_194  = 760194,
  IR_188  = 770188,
  IR_190  = 770190,
  IR_192  = 770192,
  IR_194  = 770194,
  IR_192M = 770492,
  IR_194M = 770494,
  IR_194N = 770794,
  PTXRAYS = 781000,
  PT_188  = 780188,
  PT_197  = 780197,
  PT_195M = 780495,
  AUXRAYS = 791000,
  AU_194  = 790194,
  AU_195  = 790195,
  AU_196  = 790196,
  AU_198  = 790198,
  AU_199  = 790199,
  AU_195M = 790495,
  AU_197M = 790497,
  HG_194  = 800194,
  HG_197  = 800197,
  HG_203  = 800203,
  HG_195M = 800495,
  HG_197M = 800497,
  TL_200  = 810200,
  TL_201  = 810201,
  TL_202  = 810202,
  TL_204  = 810204,
  TL_206  = 810206,
  TL_207  = 810207,
  TL_208  = 810208,
  PB_N_G  = 822000,
  PB_202  = 820202,
  PB_203  = 820203,
  PB_210  = 820210,
  PB_212  = 820212,
  PB_214  = 820214,
  PB_202M = 820502,
  BIXRAYS = 831000,
  BI_N_G  = 832000,
  BI_205  = 830205,
  BI_206  = 830206,
  BI_207  = 830207,
  BI_210  = 830210,
  BI_211  = 830211,
  BI_212  = 830212,
  BI_213  = 830213,
  BI_214  = 830214,
  PO_209  = 840209,
  PO_210  = 840210,
  AT_211  = 850211,
  FR_223  = 870223,
  RA_223  = 880223,
  RA_225  = 880225,
  RA_226  = 880226,
  AC_225  = 890225,
  AC_227  = 890227,
  THXRAYS = 901000,
  TH_227  = 900227,
  TH_228  = 900228,
  TH_229  = 900229,
  TH_231  = 900231,
  TH_232  = 900232,
  PA_231  = 910231,
  PA_233  = 910233,
  PA_234  = 910234,
  PA_234M = 910534,
  UXRAYS  = 921000,
  U_232   = 920232,
  U_233   = 920233,
  U_234   = 920234,
  U_235   = 920235,
  U_236   = 920236,
  U_237   = 920237,
  U_238   = 920238,
  U_239   = 920239,
  NP_236  = 930236,
  NP_237  = 930237,
  NP_238  = 930238,
  NP_239  = 930239,
  NP_240M = 930540,
  PUXRAYS = 941000,
  PU_236  = 940236,
  PU_238  = 940238,
  PU_239  = 940239,
  PU_240  = 940240,
  PU_241  = 940241,
  PU_242  = 940242,
  AM_241  = 950241,
  AM_243  = 950243,
  AM_242M = 950542,
  CM_243  = 960243,
  CM_244  = 960244,
  CM_254  = 960254,
  CF_249  = 980249,
  CF_251  = 980251,
  CF_252  = 980252
}

/***********************************************************************/

/* This structure holds a list of all nuclides that may be present.
The probabilities do not need to add to 1.  If a specific mixture is
suspected then it would be listed in the IdentificationSolution.  

Example:  
  Algorithm considers 4 possible solutions with associated probabilities
    Ba133 + Cs137 45%
    Pu            35%
    I131  + Cs137 12%
    Cs137          8%

  The Summary would be given as
    Cs137 65%
    Ba133 45%
    Pu    35%
    I131  12%

*/

struct NuclideSummaryEntry
{
  1: Nuclide nuclide;
  2: double probability;
  3: optional ActivityInfo activityInfo;
  4: optional DoseInfo doseInfo;  // at one meter
}

struct IdentificationSolutionEntry
{
  1: Nuclide nuclide;
  2: double countFraction;
  3: optional ActivityInfo activityInfo;
  4: optional DoseInfo doseInfo;
  5: optional ShieldingInfo shieldingInfo;
}

/* This structure represents a specific solution found to fit the measurement
   it can be a mixture.
*/
struct IdentificationSolution
{
  1: double probability;
  2: list<IdentificationSolutionEntry> nuclides;
  3: optional DoseInfo doseInfo;
  4: optional Classification classification;
}

/*
 * Configuration
 */
struct IdentificationConfiguration
{
  /* List of all the nuclides being searched for */
  1: list<Nuclide> includedNuclides;
  /* Milliseconds to collect background time */
  2: i64 backgroundTime;
}

struct IdentificationDefinition
{
  1: bool hasDoseInfo = false
  2: IdentificationConfiguration startingIdentificationConfiguration;
}

/*
 * Result
 */
struct IdentificationResult
{ 
  /** time in milliseconds used for computation of result */
  1: i64 liveTime; 
  /** effective distance assumed for this source analysis (see notes) */
  2: double distance;
  /** Millseconds since Unix Epoch - Time the decision was actually reached */
  3: i64 timeOfDecision 
  /** Information about the estimated dose */
  4: optional DoseInfo doseInfo
  /** Classification summary */
  5: list<ClassificationSummaryEntry> classificationSummaries;
  /** List of all nuclides that may be present with probabilites */
  6: list<NuclideSummaryEntry> nuclideSummaries;
  /** Specific fits to the spectrum including mixtures */
  7: optional list<IdentificationSolution>  solutions;
  /** Return spectrum used to determine result */
  8: Spectrum.SpectrumResult spectrumResult;
}