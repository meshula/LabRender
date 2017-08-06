//
//  Utils.cpp
//  LabApp
//
//  Created by Nick Porcino on 2014 04/5.
//  Copyright (c) 2014 Nick Porcino. All rights reserved.
//

#include "LabRender/Utils.h"

#include <iostream>
#include <map>
#include <string>

using namespace std;

namespace {
    
    map<string, string> variables;
    
    std::string cleanPath(const std::string& path) {
        string result = path;
        for (auto i : variables) {
            size_t pos = path.find(i.first);
            if (pos == string::npos)
                continue;
            
            result = result.replace(pos, i.first.length(), i.second);
        }
        return result;
    }
}

namespace lab {
    
    void addPathVariable(const std::string& var, const std::string& replacement) 
	{
        variables[var] = replacement;
    }
    
    std::string expandPath(char const*const path) 
	{
        return cleanPath(path);
    }
    
    std::string loadFile(char const*const path, bool errorIfNotFound) 
	{   
        string resolvedPath(path);
        resolvedPath = cleanPath(resolvedPath);
        
        FILE* f = fopen(resolvedPath.c_str(), "rb");
        if (!f) {
            if (errorIfNotFound)
                cerr << "Could not open file " << resolvedPath << endl;
            
            return "";
        }
        
        std::string result;
        fseek(f, 0, SEEK_END);
        size_t l = ftell(f);
        fseek(f, 0, SEEK_SET);
        uint8_t* data = new uint8_t[l+1];
        fread(data, 1, l, f);
        fclose(f);
        data[l] = '\0';
        result.assign((char*)data);
        delete[] data;
        return result;
    }
    
}