PixelAnnotationTool
============================

Software that allows you to manually and quickly annotate segmentation images in directories.

<img src="https://github.com/UndeadBlow/PixelAnnotationTool/raw/neural_net/neural_demo.jpg"/>

----------

Versions:

**1.5neural**

* Neural network support added
* If classes and their colors in network model file (.pb) available then will be load on the top of JSON
* Network inference can be used for annotation help
* Everything heavy (network load and inference) works async
* Added ready-to-use binary for Windows x64 and network example (lanes segmentation)
* Last network loaded will be saved and loaded on next start
* Fixed some bugs

**1.3beta**

* Added ability to load and save JSON with labels again.
* Last loaded JSON will be saved and automatically loaded on start.
* Segmentation saves and loads from color mask only, no ID mask used.
* You can navigate images with Q and E (previous and next image), even if multiply directories opened.

----------


### Building Dependencies :
* [Qt](https://www.qt.io/download-open-source/)  >= 5.10
* [CMake](https://cmake.org/download/) >= 3.0.0
* [OpenCV](http://opencv.org/releases.html) >= 3.0.0
* [Tensorflow](https://www.tensorflow.org/) >= 1.10
* For Windows Compiler : Works under Visual Studio >= 2015

### Download binaries :
Go to release [page](https://github.com/UndeadBlow/PixelAnnotationTool/releases)

### License :

GNU Lesser General Public License v3.0 

Permissions of this copyleft license are conditioned on making available complete source code of licensed works and modifications under the same license or the GNU GPLv3. Copyright and license notices must be preserved. Contributors provide an express grant of patent rights. However, a larger work using the licensed work through interfaces provided by the licensed work may be distributed under different terms and without source code for the larger work.

[more](https://github.com/abreheret/PixelAnnotationTool/blob/master/LICENSE)
