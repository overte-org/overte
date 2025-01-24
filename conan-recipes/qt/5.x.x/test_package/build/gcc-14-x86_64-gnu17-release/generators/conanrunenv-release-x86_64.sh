script_folder="/home/juliangro/git/overte/conan-recipes/qt/5.x.x/test_package/build/gcc-14-x86_64-gnu17-release/generators"
echo "echo Restoring environment" > "$script_folder/deactivate_conanrunenv-release-x86_64.sh"
for v in PATH LD_LIBRARY_PATH DYLD_LIBRARY_PATH FONTCONFIG_PATH ACLOCAL_PATH AUTOMAKE_CONAN_INCLUDES M4
do
    is_defined="true"
    value=$(printenv $v) || is_defined="" || true
    if [ -n "$value" ] || [ -n "$is_defined" ]
    then
        echo export "$v='$value'" >> "$script_folder/deactivate_conanrunenv-release-x86_64.sh"
    else
        echo unset $v >> "$script_folder/deactivate_conanrunenv-release-x86_64.sh"
    fi
done


export PATH="/home/juliangro/.conan2/p/b/qt23376165b3dee/p/bin:/home/juliangro/.conan2/p/autom480a421c82a75/p/bin:/home/juliangro/.conan2/p/autocf2af015330354/p/bin:/home/juliangro/.conan2/p/m43fe61932e2887/p/bin:/home/juliangro/.conan2/p/b/nssb1d8c7def7e78/p/bin:$PATH"
export LD_LIBRARY_PATH="/home/juliangro/.conan2/p/b/qt23376165b3dee/p/lib:/home/juliangro/.conan2/p/b/qt23376165b3dee/p/plugins/platforms:/home/juliangro/.conan2/p/b/qt23376165b3dee/p/plugins/playlistformats:/home/juliangro/.conan2/p/b/qt23376165b3dee/p/plugins/position:/home/juliangro/.conan2/p/b/qt23376165b3dee/p/plugins/geoservices:/home/juliangro/.conan2/p/b/qt23376165b3dee/p/plugins/imageformats:/home/juliangro/.conan2/p/b/qt23376165b3dee/p/plugins/iconengines:/home/juliangro/.conan2/p/b/qt23376165b3dee/p/plugins/sqldrivers:/home/juliangro/.conan2/p/b/fontcc688c5c89a284/p/lib:/home/juliangro/.conan2/p/b/nssb1d8c7def7e78/p/lib:/home/juliangro/.conan2/p/b/nsprc5d1b95cc6e17/p/lib:$LD_LIBRARY_PATH"
export DYLD_LIBRARY_PATH="/home/juliangro/.conan2/p/b/qt23376165b3dee/p/lib:/home/juliangro/.conan2/p/b/qt23376165b3dee/p/plugins/platforms:/home/juliangro/.conan2/p/b/qt23376165b3dee/p/plugins/playlistformats:/home/juliangro/.conan2/p/b/qt23376165b3dee/p/plugins/position:/home/juliangro/.conan2/p/b/qt23376165b3dee/p/plugins/geoservices:/home/juliangro/.conan2/p/b/qt23376165b3dee/p/plugins/imageformats:/home/juliangro/.conan2/p/b/qt23376165b3dee/p/plugins/iconengines:/home/juliangro/.conan2/p/b/qt23376165b3dee/p/plugins/sqldrivers:/home/juliangro/.conan2/p/b/fontcc688c5c89a284/p/lib:/home/juliangro/.conan2/p/b/nssb1d8c7def7e78/p/lib:/home/juliangro/.conan2/p/b/nsprc5d1b95cc6e17/p/lib:$DYLD_LIBRARY_PATH"
export FONTCONFIG_PATH="$FONTCONFIG_PATH:/home/juliangro/.conan2/p/b/fontcc688c5c89a284/p/res/etc/fonts"
export ACLOCAL_PATH="$ACLOCAL_PATH:/home/juliangro/.conan2/p/b/libto201aaf23b4bde/p/res/aclocal"
export AUTOMAKE_CONAN_INCLUDES="$AUTOMAKE_CONAN_INCLUDES:/home/juliangro/.conan2/p/b/libto201aaf23b4bde/p/res/aclocal"
export M4="/home/juliangro/.conan2/p/m43fe61932e2887/p/bin/m4"