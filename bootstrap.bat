
echo installing Effekseer buildroot: %1 install:%2
git clone --recursive https://github.com/effekseer/Effekseer %1/deps/Effekseer
pushd %1\deps\Effekseer
mkdir build
cd build
cmake .. -DUSE_MSVC_RUNTIME_LIBRARY_DLL=ON -DCMAKE_INSTALL_PREFIX=%2 -DBUILD_GL=ON -DBUILD_EXAMPLES=OFF -DBUILD_EDITOR=OFF -DBUILD_DX9=OFF -DBUILD_DX11=OFF -DBUILD_DX12=OFF -DBUILD_METAL=OFF -DUSE_OPENGL3=ON -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build . --target install --config RelWithDebInfo
popd

echo --------------------------------------------------------------------------
echo installing USD buildroot: %1 install:%2
git clone https://github.com/PixarAnimationStudios/USD.git %1/deps/USD
python %1/deps/USD/build_scripts/build_usd.py --no-python --no-tools --no-usdview --draco --materialx %2


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

echo --------------------------------------------------------------------------
echo add the following to the cmake configuration for LabRender:
echo    -DVFX_ROOT=%2

