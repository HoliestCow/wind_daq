
from distutils.core import setup, Extension

# Digitzer readout using digiTES

module1 = Extension('caenReadoutLib',
                    define_macros = [('MAJOR_VERSION', '1'),
                                     ('MINOR_VERSION', '0')],
                    include_dirs = ['/usr/local/include',
                                    './include',
                                    '/home/holiestcow/Documents/personal/play/CAENLib/CAENDigitizer_2.11.0/include'],
                    # libraries = ['tcl83'],
                    libraries = ['CAENDigitizer'],
                    library_dirs = ['/usr/local/lib',
                                    '/home/holiestcow/Documents/personal/play/CAENLib/CAENDigitizer_2.11.0/lib/x86_64/'],
                    sources = ['./src/BoardUtils.cpp',
                               './src/Configure.cpp',
                               './src/Console.cpp',
                               './src/DataFiles.cpp',
                               './src/digiTES.cpp',
                               './src/fft.cpp',
                               './src/Histograms.cpp',
                               './src/ParamParser.cpp',
                               './src/Plots.cpp',
                               './src/PreProcess.cpp',
                               './src/Queues.cpp',
                               './src/Readout.cpp',
                               './src/Readout.cpp',
                               './src/Statistics.cpp',
                               './src/ZCcal.cpp'])

setup (name = 'CAENReadoutLib',
       version = '1.0',
       description = 'LOVEME',
       author = 'Carl G. Britt III',
       author_email = 'yoloswag',
       long_description = '''
I hope this works.
''',
       ext_modules = [module1])
