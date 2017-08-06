//
//  TextureLoader.mm
//  LabApp
//
//  Copyright (c) 2014 Nick Porcino. All rights reserved.
//
#import <Foundation/Foundation.h>
#import <ImageIO/CGImageSource.h>

#import "TextureLoader.h"

#include "LabRender/Texture.h"
#include "LabRender/gl4.h"

namespace LabRender {

    std::unique_ptr<Texture> loadTexture(char const*const urlStr, bool asCube) {
        NSString* path = [NSString stringWithCString:urlStr encoding:NSUTF8StringEncoding];
        NSURL* url = [NSURL fileURLWithPath:path];
        CGImageSourceRef imageSource = CGImageSourceCreateWithURL((__bridge CFURLRef) url, nullptr);
        CGImageRef image = CGImageSourceCreateImageAtIndex(imageSource, 0, nullptr);
        CFRelease(imageSource);

        size_t width  = CGImageGetWidth (image);
        size_t height = CGImageGetHeight(image);
        if (width > 1024) width = 1024;
        if (height > 1024) height = 1024;
        
        CGRect rect = CGRectMake(0.0f, 0.0f, width, height);

        void *imageData = malloc(width * height * 4);
        CGColorSpaceRef colourSpace = CGColorSpaceCreateDeviceRGB();
        // supported pixel formats are documented here
        // https://developer.apple.com/library/mac/documentation/graphicsimaging/Conceptual/drawingwithquartz2d/dq_context/dq_context.html#//apple_ref/doc/uid/TP30001066-CH203-BCIBHHBB
        CGContextRef ctx = CGBitmapContextCreate(imageData, width, height,
                                                 8,             // bits per channel, on OSX 16 is also supported
                                                 width * 4,     // bytes per row
                                                 colourSpace,
                                                 kCGBitmapByteOrder32Host | kCGImageAlphaPremultipliedFirst);
        CFRelease(colourSpace);
        
        if (!asCube) {
            CGContextTranslateCTM(ctx, 0, height);
            CGContextScaleCTM(ctx, 1.0f, -1.0f);
        }
        CGContextSetBlendMode(ctx, kCGBlendModeCopy);
        CGContextDrawImage(ctx, rect, image);
        CGContextRelease(ctx);
        CFRelease(image);

        std::unique_ptr<Texture> texture(new Texture());
        
        if (asCube)
            texture->create((int)width, (int)height, Texture::Type::u8x4, GL_LINEAR, GL_CLAMP_TO_EDGE, Texture::Type::u8x4, imageData);
        else
            texture->create((int)width, (int)height, Texture::Type::u8x4, GL_LINEAR, GL_CLAMP_TO_EDGE, Texture::Type::u8x4, imageData);

        free(imageData);
        return texture;
    }
    

    std::unique_ptr<Texture> loadCubeTexture(std::string url[6]) {
        void* imageData[6];

        size_t width = 0;
        size_t height = 0;
        for (int i = 0; i < 6; ++i) {
            NSString* path = [NSString stringWithCString:url[i].c_str() encoding:NSUTF8StringEncoding];
            NSURL* url = [NSURL fileURLWithPath:path];
            CGImageSourceRef imageSource = CGImageSourceCreateWithURL((__bridge CFURLRef) url, NULL);
            if (!imageSource)
                continue;
            
            CGImageRef image = CGImageSourceCreateImageAtIndex(imageSource, 0, NULL);
            CFRelease(imageSource);

            width  = CGImageGetWidth (image);
            height = CGImageGetHeight(image);
            if (width > 1024) width = 1024;
            if (height > 1024) height = 1024;

            CGRect rect = CGRectMake(0.0f, 0.0f, width, height);

            imageData[i] = malloc(width * height * 4);
            CGColorSpaceRef colourSpace = CGColorSpaceCreateDeviceRGB();
            // supported pixel formats are documented here
            // https://developer.apple.com/library/mac/documentation/graphicsimaging/Conceptual/drawingwithquartz2d/dq_context/dq_context.html#//apple_ref/doc/uid/TP30001066-CH203-BCIBHHBB
            CGContextRef ctx = CGBitmapContextCreate(imageData[i], width, height,
                                                     8,             // bits per channel, on OSX 16 is also supported
                                                     width * 4,     // bytes per row
                                                     colourSpace,
                                                     kCGBitmapByteOrder32Host | kCGImageAlphaPremultipliedFirst);
            CFRelease(colourSpace);

            const bool flip = false;//(i==2) || (i==3);
            if (flip) {
                CGContextTranslateCTM(ctx, 0, height);
                CGContextScaleCTM(ctx, 1.0f, -1.0f);
            }
            CGContextSetBlendMode(ctx, kCGBlendModeCopy);
            CGContextDrawImage(ctx, rect, image);
            CGContextRelease(ctx);
            CFRelease(image);
        }

        std::unique_ptr<Texture> texture(new Texture());
        if (width > 0 && height > 0)
            texture->createCube((int)width, (int)height, Texture::Type::u8x4, GL_LINEAR, GL_CLAMP_TO_EDGE, Texture::Type::u8x4,
                                imageData[0], imageData[1], imageData[2], imageData[3], imageData[4], imageData[5]);

        for (int i = 0; i < 6; ++i)
            free(imageData[i]);

        return texture;
    }


} // Lab
