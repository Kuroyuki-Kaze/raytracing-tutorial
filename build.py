#!/usr/env/python3.10

import os
import argparse
import shutil
import subprocess
import pprint

build_dir = 'build'
bin_dir = 'bin'
src_dir = 'src'
include_dir = 'include'
target = 'rtc'

msvc_cl = 'cl'
msvc_flags = '/MT /nologo /Gm- /GR- /EHa /Od /Oi /WX /W4 /wd4127 /wd4701 /wd4201 /wd4100 /wd4189 /FC /Zi'
msvc_libpaths = '/LIBPATH:"C:/Program Files (x86)/Windows Kits/10/Lib/10.0.19041.0/um/x64" /LIBPATH:"C:/Program Files (x86)/Windows Kits/10/Lib/10.0.19041.0/ucrt/x64" /LIBPATH:"C:/Program Files (x86)/Microsoft Visual Studio/2019/BuildTools/VC/Tools/MSVC/14.29.30133/lib/x64" /LIBPATH:"C:/Program Files (x86)/Microsoft Visual Studio/2019/BuildTools/VC/Tools/MSVC/14.29.30133/lib/x64"'
msvc_extern_libs = 'user32.lib gdi32.lib shlwapi.lib winmm.lib'
msvc_linkflags32 = f'-opt:ref -subsystem:windows,5.1 {msvc_libpaths} {msvc_extern_libs}'
msvc_linkflags64 = f'-opt:ref {msvc_libpaths} {msvc_extern_libs}'
msvc_standard_include = 'C:/Program Files (x86)/Microsoft Visual Studio/2019/BuildTools/VC/Tools/MSVC/14.29.30133/include'
msvc_ucrt_include = 'C:/Program Files (x86)/Windows Kits/10/Include/10.0.19041.0/ucrt'
msvc_um_include = 'C:/Program Files (x86)/Windows Kits/10/Include/10.0.19041.0/um'
msvc_shared_include = 'C:/Program Files (x86)/Windows Kits/10/Include/10.0.19041.0/shared'
msvc_winrt_include = 'C:/Program Files (x86)/Windows Kits/10/Include/10.0.19041.0/winrt'
msvc_cppwinrt_include = 'C:/Program Files (x86)/Windows Kits/10/Include/10.0.19041.0/cppwinrt/winrt'
msvc_link = 'link'
msvc_link_flags = f'/OUT:{bin_dir}/{target}.exe'

cxx = 'g++'
cxxflags = '-g'
linkflags = '-Wall'


src_files = []
src_files_no_dir = []
src_files_no_dir_no_ext = []

ap = argparse.ArgumentParser(description='A script to build and run the program. Not that hard to use.')
ap.add_argument('-c', '--clean', action='store_true', help='clean build and bin folders')
ap.add_argument('-b', '--build', action='store_true', help='build the program')
ap.add_argument('-f', '--force', action='store_true', help='force build')
ap.add_argument('-r', '--run', action='store_true', help='run the program')
ap.add_argument('-d', '--debug', action='store_true', help='debug mode')
ap.add_argument('-m', '--msvc', action='store_true', help='use msvc instead of gcc')
ap.add_argument('-O', '--optimize', action='store', type=str, help='optimization level (g | 0-3)')
ap.add_argument('-Wall', '--all-warnings', action='store_true', help='enable all warnings')
ap.add_argument('-Wextra', '--extra-warnings', action='store_true', help='enable extra warnings')
ap.add_argument('-D', '--define', action='append', type=str, help='define a variable', nargs='*')

args = vars(ap.parse_args())

pprint.pprint(args)

if args['define']:
    for definition in args['define']:
        cxxflags += f' -D{definition[0]}'
        msvc_flags += f' /D{definition[0]}'

if args['optimize']:
    cxxflags += f' -O{args["optimize"]}'

if args['all_warnings']:
    cxxflags += ' -Wall'
    
if args['extra_warnings']:
    cxxflags += ' -Wextra'
    
def debug():
    src_files = []
    
    for root, _, files in os.walk(src_dir):
        for file in files:
            if file.endswith('.cpp'):
                src_files.append(os.path.join(root, file))
                
    return os.system(f'{cxx} {cxxflags} -g -o {bin_dir}/{target} {" ".join(src_files)} {linkflags}')

def create_folder():
    if not os.path.exists(build_dir):
        os.makedirs(build_dir)
    
    if not os.path.exists(bin_dir):
        os.makedirs(bin_dir)

def clean():
    shutil.rmtree(build_dir)
    shutil.rmtree(bin_dir)
        
    create_folder()

def build():
    create_folder()
    
    for root, _, files in os.walk(src_dir):
        for file in files:
            if file.endswith('.cpp'):
                if not args['force']:
                    try:
                        if os.path.getmtime(os.path.join(root, file)) > os.path.getmtime(os.path.join(build_dir, os.path.splitext(file)[0] + '.o')):
                            src_files.append(os.path.join(root, file))
                            src_files_no_dir.append(file)
                            src_files_no_dir_no_ext.append(os.path.splitext(file)[0])
                    except FileNotFoundError:
                        src_files.append(os.path.join(root, file))
                        src_files_no_dir.append(file)
                        src_files_no_dir_no_ext.append(os.path.splitext(file)[0])
                else:
                    src_files.append(os.path.join(root, file))
                    src_files_no_dir.append(file)
                    src_files_no_dir_no_ext.append(os.path.splitext(file)[0])
    
    if (len(src_files) == 0 and not args['force']):
        print("Already up to date")
        return 0

    srcs = list(zip(src_files, src_files_no_dir, src_files_no_dir_no_ext))
    if args['msvc']:
        final_build_str = f'{msvc_link} {msvc_link_flags} '
    else:
        final_build_str = f'{cxx} -pass-exit-codes -o {bin_dir}/{target} '
        

    for src in srcs:
        if args['msvc']:
            compile_error_code = os.system(f'{msvc_cl} {msvc_flags} /c {src[0]} /Fo{build_dir}/{src[2]}.o /I{include_dir} /I"{msvc_standard_include}" /I"{msvc_ucrt_include}" /I"{msvc_um_include}" /I"{msvc_shared_include}" /I"{msvc_winrt_include}" /I"{msvc_cppwinrt_include}"')
        else:
            compile_error_code = os.system(f'{cxx} {cxxflags} -c {src[0]} -o {build_dir}/{src[2]}.o -I{include_dir}')
            
        if compile_error_code:
            print("compilation error")
            return compile_error_code
        
        final_build_str += f'{build_dir}/{src[2]}.o '
        
    if args['msvc']:
        final_build_str64 = final_build_str.replace(f'{target}.exe', f'{target}64.exe', 1)
        final_build_str += msvc_linkflags32
        final_build_str64 += msvc_linkflags64
        print(final_build_str)
        print(final_build_str64)
    else:
        final_build_str += f'{linkflags}'
    
    # what the fuck, why does os.system returns 0 even when it fails
    # ec2 = subprocess.call(final_build_str, shell=True)
    if args['msvc']:
        ec1 = os.system(final_build_str)
        ec2 = os.system(final_build_str64)
    else:
        ec1 = os.system(final_build_str)
        ec2 = 0
    # print("Error code: ", ec)
    # print("Error code 2: ", ec2)
    return (ec1, ec2)
    
def run():
    error = build()
    if type(error) == tuple:
        if error[0] != 0 or error[1] != 0:
            return
    else:
        if error != 0:
            return
    
    print("finished build with no error")
    
    os.chdir(bin_dir)
    if args['msvc']:
        subprocess.call([f'./{target}64'])
    else:
        subprocess.call([f'./{target}'])
        
    
if args['debug']:
    exit(debug())
        
if args['clean']:
    clean()

if args['build']:
    build()
    
if args['run']:
    run()