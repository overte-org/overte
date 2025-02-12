script_folder="/home/juliangro/git/overte/conan-recipes/qt/5.x.x/test_package/build/gcc-14-x86_64-gnu17-release/generators"
echo "echo Restoring environment" > "$script_folder/deactivate_conanbuildenv-release-x86_64.sh"
for v in ACLOCAL_PATH AUTOMAKE_CONAN_INCLUDES AUTOCONF AUTORECONF AUTOHEADER AUTOM4TE M4
do
    is_defined="true"
    value=$(printenv $v) || is_defined="" || true
    if [ -n "$value" ] || [ -n "$is_defined" ]
    then
        echo export "$v='$value'" >> "$script_folder/deactivate_conanbuildenv-release-x86_64.sh"
    else
        echo unset $v >> "$script_folder/deactivate_conanbuildenv-release-x86_64.sh"
    fi
done


export ACLOCAL_PATH="$ACLOCAL_PATH:/home/juliangro/.conan2/p/xorg-a7814e7f591fa/p/bin/share/aclocal:/home/juliangro/.conan2/p/b/libto201aaf23b4bde/p/res/aclocal"
export AUTOMAKE_CONAN_INCLUDES="$AUTOMAKE_CONAN_INCLUDES:/home/juliangro/.conan2/p/b/libto201aaf23b4bde/p/res/aclocal"
export AUTOCONF="/home/juliangro/.conan2/p/autocf2af015330354/p/bin/autoconf"
export AUTORECONF="/home/juliangro/.conan2/p/autocf2af015330354/p/bin/autoreconf"
export AUTOHEADER="/home/juliangro/.conan2/p/autocf2af015330354/p/bin/autoheader"
export AUTOM4TE="/home/juliangro/.conan2/p/autocf2af015330354/p/bin/autom4te"
export M4="/home/juliangro/.conan2/p/m43fe61932e2887/p/bin/m4"