# Image Colorization with Disjoint-Set Data Structures
 
These C code snippets are designed to colorize black and white images in the PBM (Portable Bitmap) format. They employ disjoint-set data structures (also known as union-find data structures) to group pixels with similar colors together and subsequently assign random colors to each group. The goal is to produce colorized versions of monochromatic images.

Usage

 Begin by compiling the code using a C compiler. You can do this by executing gcc linked_list.c -o colorize1 and gcc tree.c -o colorize2 for each version.

 After compilation, run the program by providing the path to the PBM image you want to colorize as a command-line argument. For example:

./colorize1 input_image.pbm or ./colorize2 input_image.pbm

The output file will have the same name as the input file but with "_colorized.ppm" appended to it.