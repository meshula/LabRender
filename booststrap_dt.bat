
echo --------------------------------------------------------------------------
echo installing dawn and tint, also glm, glfw
echo git clone --recursive https://github.com/meshula/LabSlang.git %1/deps/LabSlang
call conda install --yes jinja2
pushd %1\deps\LabSlang
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=%2 
cmake --build . --target install --config RelWithDebInfo
popd

