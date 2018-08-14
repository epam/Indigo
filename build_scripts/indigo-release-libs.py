import os
import platform
import shutil
import subprocess
import sys

from optparse import OptionParser


check_call = None
if not hasattr(subprocess, 'check_call'):
    def check_call_replacement(*popenargs, **kwargs):
        retcode = subprocess.call(*popenargs, **kwargs)
        if retcode:
            cmd = kwargs.get("args")
            if cmd is None:
                cmd = popenargs[0]
            raise subprocess.CalledProcessError(retcode, cmd)
        return 0
    check_call = check_call_replacement
else:
    check_call = subprocess.check_call


def get_cpu_count():
    cpu_count = 1
    if os.name == 'java':
        from java.lang import Runtime
        runtime = Runtime.getRuntime()
        cpu_count = runtime.availableProcessors()
    else:
        import multiprocessing
        cpu_count = multiprocessing.cpu_count()
    return cpu_count


def build_libs(cl_args):
    presets = {
        "win32-2013": ("Visual Studio 12", ""),
        "win32-2015": ("Visual Studio 14", ""),
        "win32-2017": ("Visual Studio 15", ""),
        "win64-2013": ("Visual Studio 12 Win64", ""),
        "win64-2015": ("Visual Studio 14 Win64", ""),
        "win64-2017": ("Visual Studio 15 Win64", ""),
        "win32-mingw": ("MinGW Makefiles", "-DSUBSYSTEM_NAME=x86"),
        "win64-mingw": ("MinGW Makefiles", "-DSUBSYSTEM_NAME=x64"),
        "linux32": ("Unix Makefiles", "-DSUBSYSTEM_NAME=x86"),
        "linux32-universal": ("Unix Makefiles", "-DSUBSYSTEM_NAME=x86"),
        "linux64": ("Unix Makefiles", "-DSUBSYSTEM_NAME=x64"),
        "linux64-universal": ("Unix Makefiles", "-DSUBSYSTEM_NAME=x64"),
        "mac10.7": ("Xcode", "-DSUBSYSTEM_NAME=10.7"),
        "mac10.8": ("Xcode", "-DSUBSYSTEM_NAME=10.8"),
        "mac10.9": ("Xcode", "-DSUBSYSTEM_NAME=10.9"),
        "mac10.10": ("Xcode", "-DSUBSYSTEM_NAME=10.10"),
        "mac10.11": ("Xcode", "-DSUBSYSTEM_NAME=10.11"),
        "mac10.12": ("Xcode", "-DSUBSYSTEM_NAME=10.12"),
        "mac10.13": ("Xcode", "-DSUBSYSTEM_NAME=10.13"),
        "mac-universal": ("Unix Makefiles", "-DSUBSYSTEM_NAME=10.7"),
    }

    parser = OptionParser(description='Indigo libraries build script')
    parser.add_option('--generator', help='this option is passed as -G option for cmake')
    parser.add_option('--params', default="", help='additional build parameters')
    parser.add_option('--config', default="Release", help='project configuration')
    parser.add_option('--nobuild', default=False, action="store_true", help='configure without building', dest="nobuild")
    parser.add_option('--clean', default=False, action="store_true", help='delete all the build data', dest="clean")
    parser.add_option('--preset', type="choice", dest="preset", choices=list(presets.keys()), help='build preset %s' % (str(list(presets.keys()))))
    parser.add_option('--with-static', action='store_true', help='Build Indigo static libraries', default=False, dest='withStatic')
    parser.add_option('--verbose', action='store_true', help='Show verbose build information', default=False, dest='buildVerbose')
    parser.add_option('--cairo-gl', dest="cairogl", default=False, action="store_true", help='Build Cairo with OpenGL support')
    parser.add_option('--cairo-vg', dest="cairovg", default=False, action="store_true", help='Build Cairo with CairoVG support')
    parser.add_option('--cairo-egl', dest="cairoegl", default=False, action="store_true", help='Build Cairo with EGL support')
    parser.add_option('--cairo-glesv2', dest="cairoglesv2", default=False, action="store_true", help='Build Cairo with GLESv2 support')
    parser.add_option('--find-cairo', dest="findcairo", default=False, action="store_true", help='Find and use system Cairo')
    parser.add_option('--find-pixman', dest="findpixman", default=False, action="store_true", help='Find and use system Pixman')
    parser.add_option('--no-multithreaded-build', dest='mtbuild', default=True, action='store_false', help='Use only 1 core to build')
    if os.name == 'posix':
        parser.add_option('--check-abi', dest='checkabi', default=False, action="store_true", help='Check ABI type of Indigo libraries on Linux')

    (args, left_args) = parser.parse_args(cl_args)
    if len(left_args) > 0:
        print("Unexpected arguments: %s" % (str(left_args)))
        exit()

    auto_vs = False
    if args.preset:
        args.generator, args.params = presets[args.preset]
    else:
        if os.name == 'java':
            from java.lang import System
            system = System.getProperty("os.name")
        else:
            system = platform.system()

        if system in ('Darwin', 'Mac OS X'):
            mac_version = platform.mac_ver()[0] if os.name != 'java' else System.getProperty('os.version')
            preset = 'mac{}'.format('.'.join(mac_version.split('.')[:2]))
        elif system == 'Linux':
            preset = 'linux{}'.format(platform.architecture()[0][:2])
        elif system == 'Windows':
            preset = ''
            auto_vs = True
        else:
            raise NotImplementedError('Unsupported OS: {}'.format(system))
        if preset:
            print("Auto-selecting preset: {}".format(preset))
            args.generator, args.params = presets[preset]
        else:
            print("Preset is no selected, continuing with empty generator and params...")
            args.generator, args.params = '', ''

    cur_dir = os.path.abspath(os.path.dirname(__file__))
    root = os.path.join(cur_dir, "..")
    project_dir = os.path.join(cur_dir, "indigo-all")

    if args.cairogl:
        args.params += ' -DWITH_CAIRO_GL=TRUE'

    if args.cairovg:
        args.params += ' -DWITH_CAIRO_VG=TRUE'

    if args.cairoegl:
        args.params += ' -DWITH_CAIRO_EGL=TRUE'

    if args.cairoglesv2:
        args.params += ' -DWITH_CAIRO_GLESV2=TRUE'

    if args.findcairo:
        args.params += ' -DUSE_SYSTEM_CAIRO=TRUE'

    if args.findcairo:
        args.params += ' -DUSE_SYSTEM_PIXMAN=TRUE'

    if args.withStatic:
        args.params += ' -DWITH_STATIC=TRUE'

    if args.preset and args.preset.find('universal') != -1:
        args.params += ' -DUNIVERSAL_BUILD=TRUE'

    if os.name == 'posix' and args.checkabi:
        args.params += ' -DCHECK_ABI=TRUE'

    build_dir = (args.generator + " " + args.config + args.params.replace('-D', ''))
    build_dir = "indigo_" + build_dir.replace(" ", "_").replace("=", "_").replace("-", "_")

    full_build_dir = os.path.join(root, "build", build_dir)

    if os.path.exists(full_build_dir) and args.clean:
        print("Removing previous project files")
        shutil.rmtree(full_build_dir)
    if not os.path.exists(full_build_dir):
        os.makedirs(full_build_dir)

    os.chdir(full_build_dir)

    environment_prefix = ''
    if args.preset and (args.preset.find('linux') != -1 and args.preset.find('universal') != -1):
        if args.preset.find('32') != -1:
            os.environ['LD_FLAGS'] = '{} }'.format(os.environ.get('LD_FLAGS', ''), '-m32')
        environment_prefix = 'CC=gcc CXX=g++'
    command = "%s cmake %s %s %s" % (environment_prefix, '-G \"%s\"' % args.generator if args.generator else '', args.params, project_dir)
    print(command)
    check_call(command, shell=True)

    if args.nobuild:
        exit(0)

    for f in os.listdir(full_build_dir):
        path, ext = os.path.splitext(f)
        if ext == ".zip":
            os.remove(os.path.join(full_build_dir, f))

    if args.generator in {"Unix Makefiles", 'MinGW Makefiles'}:
        make_args = ''
        if args.buildVerbose:
            make_args += ' VERBOSE=1'

        if args.mtbuild:
            make_args += ' -j{} '.format(get_cpu_count())

        make_command = 'mingw32-make' if args.generator == 'MinGW Makefiles' else 'make'
        check_call("%s package %s" % (make_command, make_args), shell=True)
        check_call("%s install" % (make_command), shell=True)
    elif args.generator == "Xcode":
        check_call("cmake --build . --target package --config %s" % (args.config), shell=True)
        check_call("cmake --build . --target install --config %s" % (args.config), shell=True)
    elif args.generator.startswith("Visual Studio") or auto_vs:
        vsenv = os.environ
        if args.mtbuild:
            vsenv = dict(os.environ, CL='/MP')
        check_call("cmake --build . --target PACKAGE --config %s" % (args.config), env=vsenv, shell=True)
        check_call("cmake --build . --target INSTALL --config %s" % (args.config), shell=True)
    else:
        print("Do not know how to run package and install target")
    check_call("ctest -V --timeout 60 -C %s ." % (args.config), shell=True)
    os.chdir(root)
    if not os.path.exists("dist"):
        os.mkdir("dist")
    dist_dir = os.path.join(root, "dist")

    zip_path_vec = []
    for f in os.listdir(full_build_dir):
        path, ext = os.path.splitext(f)
        if ext == ".zip":
            zip_path = os.path.join(dist_dir, f)
            shutil.copy(os.path.join(full_build_dir, f), zip_path)
            zip_path_vec.append(zip_path)

    return(zip_path_vec)


if __name__ == '__main__':
    build_libs(sys.argv[1:])
