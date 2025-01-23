script_folder="/home/juliangro/git/overte/conan-recipes/nss/all/test_package/build/gcc-14-x86_64-gnu17-release/generators"
echo "echo Restoring environment" > "$script_folder/deactivate_conanrunenv-release-x86_64.sh"
for v in PATH LD_LIBRARY_PATH DYLD_LIBRARY_PATH
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


export PATH="/home/juliangro/.conan2/p/b/nssb1d8c7def7e78/p/bin:$PATH"
export LD_LIBRARY_PATH="/home/juliangro/.conan2/p/b/nssb1d8c7def7e78/p/lib:/home/juliangro/.conan2/p/b/nsprc5d1b95cc6e17/p/lib:$LD_LIBRARY_PATH"
export DYLD_LIBRARY_PATH="/home/juliangro/.conan2/p/b/nssb1d8c7def7e78/p/lib:/home/juliangro/.conan2/p/b/nsprc5d1b95cc6e17/p/lib:$DYLD_LIBRARY_PATH"