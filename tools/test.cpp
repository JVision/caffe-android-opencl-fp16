//
//  main.m
//  metal_mac
//
//  Created by Tec GSQ on 27/11/2017.
//  Copyright © 2017 Tec GSQ. All rights reserved.
//

#include <iostream>
#include "caffe/caffe.hpp"
#include <stdio.h>
#include <stdlib.h>


#define MAX_SOURCE_SIZE (0x100000)

#define RGB_COMPONENT_COLOR 255

 
typedef struct {
     unsigned char red,green,blue;
} PPMPixel; 

typedef struct {
     int x, y;
     PPMPixel *data;
} PPMImage;


static PPMImage *readPPM(const char *filename) {
  char buff[16];
  FILE *fp;
  int c, rgb_comp_color;
  PPMImage *img;

 //open PPM file for reading
  
  fp = fopen(filename, "rb");
  if (!fp) {
    fprintf(stderr, "Unable to open file '%s'\n", filename);
    exit(1);
  }

  //read image format
  if (!fgets(buff, sizeof(buff), fp)) {
    perror(filename);
    exit(1);
  }  

  //check the image format
  if (buff[0] != 'P' || buff[1] != '6') {
       fprintf(stderr, "Invalid image format (must be 'P6')\n");
       exit(1);
  }

  //alloc memory form image
  img = (PPMImage *)malloc(sizeof(PPMImage));
  if (!img) {
       fprintf(stderr, "Unable to allocate memory\n");
       exit(1);
  }

  //check for comments
  c = getc(fp);
  while (c == '#') {
  while (getc(fp) != '\n') ;
       c = getc(fp);
  }

  ungetc(c, fp);
  //read image size information
  if (fscanf(fp, "%d %d", &img->x, &img->y) != 2) {
       fprintf(stderr, "Invalid image size (error loading '%s')\n", filename);
       exit(1);
  }

    //read rgb component
    if (fscanf(fp, "%d", &rgb_comp_color) != 1) {
         fprintf(stderr, "Invalid rgb component (error loading '%s')\n", filename);
         exit(1);
    }

    //check rgb component depth
    if (rgb_comp_color!= RGB_COMPONENT_COLOR) {
         fprintf(stderr, "'%s' does not have 8-bits components\n", filename);
         exit(1);
    }

    while (fgetc(fp) != '\n') ;
    //memory allocation for pixel data
    img->data = (PPMPixel*)malloc(img->x * img->y * sizeof(PPMPixel));

    if (!img) {
         fprintf(stderr, "Unable to allocate memory\n");
         exit(1);
    }

    //read pixel data from file
    if (fread(img->data, 3 * img->x, img->y, fp) != img->y) {
         fprintf(stderr, "Error loading image '%s'\n", filename);
         exit(1);
    }
    fclose(fp);
    return img;
}

caffe::Net<float> *_net;

int main(int argc, char** argv) {

    caffe::CPUTimer timer;
    
    caffe::Caffe::Get().set_mode(caffe::Caffe::GPU);


    timer.Start();
    _net = new caffe::Net<float>("./examples/style_transfer/style.prototxt", caffe::TEST);
    // _net->CopyTrainedLayersFrom("./fp16.caffemodel");
    _net->CopyTrainedLayersFrom("./examples/style_transfer/a1.caffemodel");
    timer.Stop();
    

    // std::string fp16_protofile = "./fp16.caffemodel";




    PPMImage *image;
    image = readPPM("./examples/style_transfer/HKU.ppm");

    caffe::Blob<float> *input_layer = _net->input_blobs()[0];
    float* converter = input_layer->mutable_cpu_data();

    for (int y = 0; y < input_layer->width(); y++) {
      for (int x = 0; x < input_layer->width(); x++) {
        converter[y * input_layer->width() + x] = image->data[y * input_layer->width() + x].red;
        converter[y * input_layer->width() + x + input_layer->width() * input_layer->width()] = image->data[y * input_layer->width() + x].green;
        converter[y * input_layer->width() + x + 2 * input_layer->width() * input_layer->width()] = image->data[y * input_layer->width() + x].blue;
      }
    }


    
    timer.Start();
    _net->Forward();
    timer.Stop();
    
    std::cout << "The time used is " << timer.MicroSeconds() << std::endl;
    
    
    
    caffe::Blob<float> *output_layer = _net->output_blobs()[0]; 


    converter = output_layer->mutable_cpu_data();




    FILE *f = fopen("./examples/style_transfer/output.ppm", "wb");
    fprintf(f, "P6\n%i %i 255\n", input_layer->width(), input_layer->width());
    for (int y = 0; y < input_layer->width(); y++) {
        for (int x = 0; x < input_layer->width(); x++) {
            fputc(converter[y * input_layer->width() + x], f);   // 0 .. 255
            fputc(converter[y * input_layer->width() + x + input_layer->width() * input_layer->width()], f); // 0 .. 255
            fputc(converter[y * input_layer->width() + x + 2 * input_layer->width() * input_layer->width()], f);  // 0 .. 255
        }
    }
    fclose(f);
    
    caffe::Caffe::Get().DeviceQuery();
    
}
