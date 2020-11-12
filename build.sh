#!/bin/bash

compiler=""
compiler_path=""
install=""
install_path=""
build_example=""
build_type="Release"
cmake_arg=""

fun_do_config()
{
    current_pwd=`pwd`
    mkdir -p $current_pwd/build $current_pwd/build/bin $current_pwd/build/lib
    cd build
}

fun_do_help()
{
    echo "usage: $0 [-i install path] [-c compiler / compiler path] [-t build type] [-e]"
    echo "  install path: install doralib path"
    echo "  compiler: specify the compiler you are using, default: gcc"
    echo "  compiler path: specify the compiler path you are using"
    echo "  build type: specify the build type, default: release "
    echo "  eg:"
    echo "      ./build.sh"
    echo "      ./build.sh -i"
    echo "      ./build.sh -i /usr/lib/"
    echo "      ./build.sh -carm-linux-gnueabihf-gcc"
    echo "      ./build.sh -c/usr/bin/arm-linux-gnueabihf-gcc"
    echo "      ./build.sh -e"
    echo "      ./build.sh -t"
    echo "      ./build.sh -tdebug"
    echo "      ./build.sh -trelease"
    echo "      ./build.sh -i -e -t"
}

fun_do_install()
{
    install="true"
    if [ " $1" != " " ]; then
        install_path=$1
    fi
}

fun_do_compiler()
{
    if [ " $1" != " " ]; then
        compiler=$1
    fi
}

fun_do_example()
{
    build_example="true"
}

fun_do_build_type()
{
    if [ " $1" != " " ]; then
        build_type=$1
    fi
}

fun_do_check()
{
    if [ " $compiler" != " " ]; then
        compiler_path=$(which $compiler)
        if [ " $compiler_path" == " " ]; then
            echo -e "\033[31mNo $compiler compiler found in the system\033[0m"
            exit
        fi
    fi

    echo "compiler : $compiler"
    echo "compiler_path : $compiler_path"
    echo "install : $install"
    echo "install_path : $install_path"
    echo "build_type : $build_type"
    echo "build_example : $build_example"
}

fun_cmake_arg_init()
{
    if [ " $compiler_path" != " " ]; then
        cmake_arg="-DCMAKE_CXX_COMPILER=$compiler_path $cmake_arg";
    fi

    if [ " $install_path" != " " ]; then
        cmake_arg="-DCMAKE_INSTALL_PREFIX=$install_path $cmake_arg";
    fi

    if [ " $build_type" != " " ]; then
        cmake_arg="-DCMAKE_BUILD_TYPE=$build_type $cmake_arg";
    fi

    cmake .. $cmake_arg;

}

fun_do_make()
{
    fun_cmake_arg_init;

    if [ " $install" != " " ]; then
        sudo make install
    else
        make
    fi

    if [ " $build_example" != " " ]; then
        mkdir -p $current_pwd/build/example
        cd $current_pwd/build/example
        cmake ../../example/. $cmake_arg;
        make
    fi
}

main()
{
    fun_do_config;
    
    # [-h] [-e] [-i install path] [-c compiler path]
    ARGS=`getopt -o hei::c::t:: --long help,example,install::,compiler::,type:: -- "$@"`
    if [ $? != 0 ] ; then echo "Terminating..." >&2 ; exit 1 ; fi
    eval set -- "$ARGS"

    while true;do
        case "$1" in
            -i | --install)
                fun_do_install $2;
                shift 2
                ;;
            -c | --compiler)
                fun_do_compiler $2;
                shift 2
                ;;
            -t | --type)
                fun_do_build_type $2;
                shift 2
                ;;
            -e | --example)
                fun_do_example;
                shift
                ;;
            -h | --help)
                fun_do_help;
                shift
                exit 0
                ;;
            --)
                shift
                break
                ;;
            *) 
                echo "unknow : {$1}"
                exit 1
                ;;
        esac
    done

    
    fun_do_check;
    fun_do_make;
}

main "$@" 
