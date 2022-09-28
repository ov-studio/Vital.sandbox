set vendor=C:\Users\Tron\Documents\GITs\Vital.sandbox\Vendor\openssl

nmake clean
perl Configure VC-WIN64A --debug --prefix=%vendor%\x64\Debug --openssldir=%vendor%\.build no-shared
nmake test
nmake install

nmake clean
perl Configure VC-WIN64A --prefix=%vendor%\x64\Release --openssldir=%vendor%\.build no-shared
nmake test
nmake install