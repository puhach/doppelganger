# Doppelganger

This face recognition application allows the identification of a person by their picture. It currently supports face recognition using the OpenFace model and a variation of the ResNet neural network. The architecture of the program is design to be scalable and anticipate the addition of new algorithms in future.

The usage details are described below.

## Set Up

It is assumed that OpenCV 4.x, C++17 compiler, and cmake 3.0 or newer are installed on the system.

### Project structure

The project has the following directory structure:
```
│   README.md
│   
├───dataset
│           
├───dlib
│               
├───models
│       dlib_face_recognition_resnet_model_v1.dat
│       nn4.v2.t7
│       shape_predictor_5_face_landmarks.dat
│       shape_predictor_68_face_landmarks.dat
│       
├───src
│   │   .gitignore
│   │   CMakeLists.txt
│   │   dlibfaceextractor.h
│   │   dlibmatrixdistancel2.h
│   │   dlibmatrixhash.h
│   │   facedb.h
│   │   facedescriptorcomputer.h
│   │   faceextractorhelper.h
│   │   labeldata.cpp
│   │   labeldata.h
│   │   main.cpp
│   │   opencvmatdistancel2.h
│   │   openface.cpp
│   │   openface.h
│   │   openfacedescriptorcomputer.h
│   │   openfacedescriptormetric.h
│   │   openfaceextractor.h
│   │   resnet.h
│   │   resnetfacedescriptorcomputer.h
│   │   resnetfacedescriptormetric.h
│   │   
│   └───build
│                           
├───test
│       readme.txt
│       shashikant-pedwal.jpg
│       sofia-solares.jpg
│       
└───writeup
        doppelganger.pdf   
        
```

### Dataset

The `dataset` directory is supposed to contain subfolders whose names correspond to labels. Image files inside each subfolder are considered to have a label of the folder. 

The project was tested on a subset of the Celebrity Together Dataset containing 5 face images of around 1100 celebrities.

```
├───dataset
│   │   readme.txt
│   │   
│   ├───n00000001
│   │       n00000001_00000263.JPEG
│   │       n00000001_00000405.JPEG
│   │       n00000001_00000412.JPEG
│   │       n00000001_00000583.JPEG
│   │       n00000001_00000900.JPEG
│   │       
│   ├───n00000003
│   │       n00000003_00000386.JPEG
│   │       n00000003_00000488.JPEG
│   │       n00000003_00000514.JPEG
│   │       n00000003_00000670.JPEG
│   │       n00000003_00000922.JPEG
│          
│   ...
│   │       
│   │       
│   │       
│   └───n00002619
│           n00002619_00000265.JPEG
│           n00002619_00000394.JPEG
│           n00002619_00000414.JPEG
│           n00002619_00000497.JPEG
│           n00002619_00000506.JPEG

```

By default, the `dataset` folder is automatically copied to the target directory when project building completes. It can be disabled by setting the `COPY_DATASET` option off in CMake (see "Build the Project" section).

### Dlib

Dlib C++ library can be downloaded from dlib.net. The project was tested with Dlib 19.21.

After downloading extract the archive to the `dlib` folder in the project root (see the directory structure above). 

### Models

This project relies on the following models:

* 5 and 68 facial landmark predictors (consult [this](https://github.com/davisking/dlib-models#shape_predictor_5_face_landmarksdatbz2) page for download links)
* ResNet face recognition [model](https://github.com/davisking/dlib-models#dlib_face_recognition_resnet_model_v1datbz2)
* OpenFace [model](https://storage.cmusatyalab.org/openface-models/nn4.v2.t7)

Place all these models inside the `models` folder (see the directory structure above).

By default, the `models` folder is automatically copied to the target directory when project building completes. It can be disabled by setting the `COPY_MODELS` option off in CMake (see "Build the Project" section).

### Specify OpenCV_DIR in CMakeLists

Open CMakeLists.txt and set the correct OpenCV directory in the following line:

```
set(OpenCV_DIR /opt/opencv/4.5.1/installation/lib/cmake/opencv4)
```

Depending on the platform and the way OpenCV was installed, it may be needed to provide the path to cmake files explicitly. On my KUbuntu 20.04 after building OpenCV 4.5.1 from sources the working `OpenCV_DIR` looks like <OpenCV installation path>/lib/cmake/opencv4. On Windows 8.1 after installing a binary distribution of OpenCV 4.2.0 it is C:\OpenCV\build.


### Build the Project

In the `src` folder create the `build` directory unless it already exists. In the simples case the project can be configured by running the following lines from the terminal:

```
cd build
cmake ..
```

This will generate all files necessary for building the project. By default the project is configured to utilize parallel execution policy and copy the dataset, the models, and the test files to the target directory when building completes. Parallel execution improves performance, but GCC compiler requires explicit linking with the TBB library to make it work. Since TBB is normally installed for building OpenCV, it should not require any additional efforts. Otherwise, consider installing `libtbb-dev` manually or disable parallel execution:

```
cmake .. -DPARALLEL_EXECUTION=OFF
```

In this case input files will be processed sequentially, so you may expect longer execution time.

Automatic copying of the dataset, models, and test files can be turned off as shown below:
```
cmake .. -DCOPY_DATASET=OFF -DCOPY_MODELS=OFF -DCOPY_TEST_DATA=OFF
```

Since most modern processors support Advanced Vector Extensions, it makes sense to set the `USE_AVX_INSTRUCTIONS` option on:

```
cmake .. -DUSE_AVX_INSTRUCTIONS=ON
```

I can't give any recommendations regarding the `DLIB_USE_CUDA` option, because on my machine Dlib does not work with CUDA at all.

When configuration is done, compile the code:

```
cmake --build . --config Release
```


## Usage

The program has to be run from the command line. It takes in the path to the image file and several optional parameters: 

```
doppelganger 
		--database=<dataset directory or cached database file>
		[--cache=<cache file (output)>]
		[--query=<image file>]
		[--tolerance=<a positive float>]
		[--algorithm=<ResNet or OpenFace>]
		[--help]
```

Parameter    | Meaning 
------------ | --------------------------------------
help, ? | Prints the help message.
input | The input image file.
output | If not empty, specifies the output file where the output image will be saved to.
lipstick_color | The new lipstick color in RRGGBBAA notation. If empty, the lipstick filter will not be applied.
eye_color | The new iris color in RRGGBBAA notation. If empty, the eye color filter will not be applied.

The RRGGBBAA color notation is similar to the one used in CSS, but you don't need to prepend it with the hash sign. The first 6 digits define the values 
of R, G, B components. The last pair of digits, interpreted as a hexadecimal number, specifies the alpha channel of the color, where 00 represents a fully transparent color and FF represents a fully opaque color. In case the alpha component is omitted, it is assumed to be FF. For example, FF000070 specifies a pure semi-transparent red color. FF0000 and ff0000ff are identical and specify a fully opaque red color.

Applying lipstick example:
```
./vimaku --input=./images/girl-no-makeup.jpg --lipstick_color=FF000050 
```

This will add a mild red lipstick effect:

![Applying lipstick](./assets/lipstick.jpg)

For realistic look I recommend alpha values from 30 to 70.


Changing eye color example:
```
./vimaku --input=./images/girl7.png --eye_color=2E1902CC  
```

This will change the iris color to brown:
![Brown eyes](./assets/eye_color_brown.jpg)

Recommended values of alpha for the eye color filter depend on the original iris color and the intensity of the new color. When eyes are originally light, the alpha values should normally be less than 70.  

Filters can be applied together:
```
./vimaku --input=./images/girl5_small.jpg --eye_color=4b724882 --lipstick_color=ff7f5050 --ouput=out.jpg  
```

This will change the iris color to blue and the lipstick color to orange. The output image will be saved to out.jpg. 

![Blue eyes and orange lipstick](./assets/blue_eyes_and_orange_lipstick.jpg)


## Credits

Images have been downloaded from pinterest.com.
