# Get Started

## 1. Install Dependencies

Your first need to install some dependencies to correctly compile Cengine source code.

### SDL2

- libsdl2-dev - Simple DirectMedia Layer development files
- libsdl2-dev - Image loading library for Simple DirectMedia Layer 2, development files
- libsdl2-ttf-dev - TrueType Font library for Simple DirectMedia Layer 2, development files

In Ubuntu you can simply install the packages from the repos by typing:

`sudo apt-get install libsdl2-dev libsdl2-dev libsdl2-ttf-dev`

### Pthreads

If the pthreads library is not installed on your system, install it (in Ubuntu) by running:

`sudo apt-get install libpthread-stubs0-dev`

## 2. Download Cengine Source Code

You can donwload Cengine's latest source from this repo. If you want the latest rolling release in master, you can git clone in the master branch. However if you want the latest stable version, please refear to the releases tabs in this repo and select the version you want.

## 3. Get Welcome Example Running

Now that you have Cengine's source code, you can try running the base example.

Let's start with a little explanation on the source code layout. You need to focus in two main directories, the src and the include one.

- src -> this is where all the .c source files are located. They are divided into different categories (modules)
- include -> this is where the .h header files are located. They are divided in the same categories with matching names as the ones located in the src directory

In addition, you will also find the assets folder, whith some Cengine default assets, you can use them in or your project or not.

Finally there is the examples directory, where you will find some examples to get you started.

We will focus on compiling the welcome example (welcome.c). To do this, in the Cengine code that you downloaded, copy the welcome.c file form the examples directory into the src directory. Now you can compile it using the included Makefile.

The are three basic commands for the Makefile:

| Command        | Description   |
| -------------  |:-------------:|
| make           | Compile your source code and tell you if there are some errors   |
| make run       | Run the compilled binary                                         |
| make clean     | Clean created objects (.o files) and binaries                    |

You will see that a window like this gets created:

![Welcome Example](./img/cengine-welcome-example.png)