# httpdownload

httpdownload is a test applicaiton used to download a file in one 1M chunks. It is a start of what could be a multithreaded application
intended for network testing. The idea is that the program will be able to satuate a network connection. Each thread would be
responsible for downloading a chunk of code and then assembling them at the end into the original file.

The code uses the POCO library for the http protocol implementation. The only reason POCO is being used is because it is what I'm familar with
and it runs on Linux, Windows, and MacOSX without modification.

To build the code a version 3.0 or better of cmake must be installed. Also required is a reasonably new compiler. This application has been tested
on MACOSX and should support Linux. While Windows might work, it has not been a consideration and your mileage may vary.

To build the code, do:
1) Clone the rep:
     git clone https://github.com/mbuckaway/httpdownload.git
2) Build poco. POCO has been setup as submodule and for this small app we are using their master.
     bash buildpoco.sh

Poco needs only be built once. Most errors, if they are can be ignored. Only the Foundation and Net libs are used. All bins are installed
in poco/bin. No attempt be been make to fix the library rpath on MacOSX.

3) Run the build.sh:
     bash build.sh

4) Run the app: ./httpdownload -h

Enjoy.


Assumptions

Most of the assumptions are documented in the code. However, there are lots of opportunities for improvement, including
multi-theaded support, better use of temporary files (100 chunks means 100 temp files), etc.. However, it is beyond the scope
of the application.

