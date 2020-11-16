#!/bin/bash

# info
author="jiejie"
email="jiejietop@gmail.com"
github="https://github.com/jiejieTop"
description="is a library"

# variable
package_dir=""
package_name=""
info_name=""
major_version="1"
minor_version="00"
version="$major_version.$minor_version"
old_version="${version}"

fun_do_generate_depend()
{
date=`date`
changelog_file="author : ${author}
VERSION : ${version}
DATE: ${date}
"

copyright_file="
******************************************************************
  * @attention
  *
  * github  : ${github}
  * author  : ${author}  
  *
* * ******************************************************************
"

postinst_file="#!/bin/sh
echo '******************************************************************'
echo 'welcome to use ${info_name}!'
echo '******************************************************************'
"
control_file="Source: ${author}
Package: ${info_name}
Version: ${version}
Section: devel
Priority: optional
Architecture: all
Depends : 
Maintainer: ${author} ${email} 
Description: ${description}
"

    echo "$control_file" > $package_dir/DEBIAN/control
    echo "$changelog_file" > $package_dir/DEBIAN/changelog
    echo "$copyright_file" > $package_dir/DEBIAN/copyright
    echo "$postinst_file" > $package_dir/DEBIAN/postinst
}

fun_do_load_version() 
{
    if [ -f "${package_dir}/DEBIAN/.version" ]; then
        while read tmp
        do
            old_version=${tmp}
            echo $old_version
        done < "${package_dir}/DEBIAN/.version"
    else
        fun_do_generate_depend
    fi

    # automatically update version information
    version=$(echo "${old_version}+0.01"|bc)
    echo $version
    echo "${version}" > "${package_dir}/DEBIAN/.version"
}

fun_do_config()
{
    if [ " $1" == " " ]; then
        echo "the construction path of the deb package must be specified"
        echo "$0 [path]"
        exit -1;
    fi
    
    package_dir=$1

    if [ " $2" == " " ]; then
    # take the name of the deb package according to the path
        package_name=${package_dir##*/}.deb
    else
        package_name=$2
    fi

    info_name=${package_name%.*}

    if [ ! -d "${package_dir}/DEBIAN" ]; then
        mkdir -p ${package_dir}/DEBIAN
        fun_do_generate_depend
        echo "please create a simulated root directory in ${package_dir}, and put the content in this directory"
        exit -1;
    fi

    echo "build $package_name in $package_dir"

}

fun_do_make_deb() 
{
    sudo chmod 775 $package_dir/DEBIAN/postinst
    dpkg -b $package_dir $package_name
}


main()
{
    fun_do_config $1 $2
    fun_do_load_version
    fun_do_generate_depend
    fun_do_make_deb
}

main "$@" 
