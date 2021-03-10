# Doppelganger

This face recognition application allows the identification of a person by their picture. It currently supports face recognition using the OpenFace model and a variation of the ResNet neural network. The architecture of the program is designed to be scalable and anticipate the addition of new algorithms in future.

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

In the `src` folder create the `build` directory unless it already exists. In the simplest case the project can be configured by running the following lines from the terminal:

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

The program has to be run from the command line. It takes in a path to an image file and several optional parameters: 

```
doppelganger --database=<dataset directory or cached database file>			
			[--cache=<cache file (output)>]
			[--query=<image file>]
			[--tolerance=<a positive float>]
			[--algorithm=<ResNet or OpenFace>]
			[--help]
```

Parameter    | Meaning 
------------ | --------------------------------------
help, ? | Prints the help message.
database | The path to a dataset directory or a cached file of previously computed face descriptors. If a directory is specified, the database will be created by processing files in that directory. In case the path specifies a file, the database will be loaded from that file. The type of descriptors stored in the file must match currently used algorithm. 
cache | If not empty, specifies the output file path where face descriptors will be saved to.
query | If not empty, specifies the path to an image of a person that needs to be recognized.
tolerance | Defines the largest allowed difference between two faces considered the same (0.7 by default).
algorithm | Specifies face recognition algorithm to use (ResNet or OpenFace). Defaults to ResNet.


The following example shows how to recognize a person in the input file `./test/shashikant-pedwal.jpg` using the ResNet neural network and the dataset of face images:

```
./doppelganger --database=./dataset --cache=resnet.db --query=./test/shashikant-pedwal.jpg --algorithm=resnet
```

It may take a couple of minutes to process all files in the dataset directory. Then it saves the descriptors into the `resnet.db` file, so we don't have to build the database again. The person in the test file will be recognized as Amitabh Bachchan.

If we want to identify another person, we can now load the database which works much faster:
```
./doppelganger --database=resnet.db --query=./test/sofia-solares.jpg --algorithm=resnet
```

It is important to note that the algorithm used for building the database must match the currently used algorithm. 

To use a different face recognition algorithm, we have to create the database again:
```
./doppelganger --database=./dataset --cache=openface.db --algorithm=openface
```

When it is ready, we can load it and perform face recognition the same way as we did with the ResNet model:
```
./doppelganger --database=openface.db --query=./test/sofia-solares.jpg --algorithm=openface
```

Please note that the default face recognition algorithm is ResNet, therefore the following will not work:
```
./doppelganger --database=openface.db --query=./test/sofia-solares.jpg 
```

It results in an error, because this database contains face descriptors computed by the OpenFace model which are different from ResNet descriptors:
```
The database file was saved for another descriptor type.
```

This will work fine:
```
./doppelganger --database=resnet.db --query=./test/sofia-solares.jpg
```

