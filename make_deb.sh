#!/bin/bash

work_dir=`pwd`


compiler=""
compiler_path=""
install=""
install_path="/usr/local"
build_example=""
build_type="Release"
build_arg=""
deb_install=""

fun_do_set_output()
{
    if [ " $work_dir" != " " -a "${work_dir}" != "/" ]; then
        mkdir -p "${work_dir}/doralib-deb/DEBIAN"
        mkdir -p "${work_dir}/doralib-deb/${install_path}"
    else
        echo "$work_dir is not a directory"
        exit -1
    fi
}

fun_do_del_output()
{
    if [ " $work_dir" != " " -a "${work_dir}" != "/" ]; then
        sudo rm -rdf "${work_dir}/doralib-deb/"
    fi
}


fun_do_help()
{
    echo "usage: $0 [-i install path] [-c compiler / compiler path] [-t build type] [-s <on> <off>] [-e]"
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
    echo "      ./build.sh -soff"
    echo "      ./build.sh -son"
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

fun_do_config_shared()
{
    if [ " $1" != " " ]; then
        build_shared=$1
    fi
}


fun_do_build_lib() 
{
    ./build.sh ${build_arg}
}

fun_do_make_deb()
{
    ./build_deb.sh "${work_dir}/doralib-deb/" "doralib.deb"
}


fun_do_arg_init()
{
    if [ " $compiler_path" != " " ]; then
        build_arg="-c${compiler_path} ${build_arg}";
    fi

    if [ " $install_path" != " " ]; then
        build_arg="-i${work_dir}/doralib-deb${install_path} ${build_arg}";
    fi

    if [ " $build_type" != " " ]; then
        build_arg="-t${build_type} ${build_arg}";
    fi

    if [ " $build_shared" != " " ]; then
        build_arg="-s${build_shared} ${build_arg}";
    fi

    echo "${build_arg}";
}

main()
{
    ARGS=`getopt -o hi::c::t::s:: --long help,install::,compiler::,type::shared:: -- "$@"`
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
            -s | --shared)
                fun_do_config_shared $2;
                shift 2
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

    fun_do_set_output
    fun_do_arg_init
    fun_do_build_lib
    fun_do_make_deb
    # fun_do_del_output
}

main "$@" 
