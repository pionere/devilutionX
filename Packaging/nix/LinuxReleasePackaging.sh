mkdir ./build/package
find build/_CPack_Packages/Linux/7Z/ -name 'devilutionx' -exec cp "{}" ./build/devilutionx \;
cp ./build/devilutionx ./build/package/devilutionx
cp ./Packaging/resources/devilutionx.mpq ./build/package/devilutionx.mpq
cp ./Packaging/resources/devilx.mpq ./build/package/devilx.mpq
cp ./build/devilutionx*.deb ./build/package/devilutionx.deb
cp ./build/devilutionx*.rpm ./build/package/devilutionx.rpm
cp ./Packaging/nix/README.txt ./build/package/README.txt
cd ./build/package/ && tar -cavf ../../devilutionx.tar.xz * && cd ../../