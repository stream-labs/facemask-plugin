# ==============================================================================
# Copyright (C) 2017 General Workings Inc
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
# ==============================================================================


# ==============================================================================
# IMPORTS
# ==============================================================================
import sys, subprocess, os, json, uuid


def fixpath(p):
    p = p.replace("\\", "/")
    b = p.split("/")
    for i in range(0, len(b)):
        if " " in b[i]:
            b[i] = '"' + b[i] + '"'
    return "\\".join(b)


# ==============================================================================
# FILE LOCATIONS
# ==============================================================================
SVNBIN = os.path.abspath(os.path.join("c:\\", '"Program Files"', "TortoiseSVN", "bin", "svn.exe"))
MASKMAKERBIN = fixpath(os.path.abspath("./maskmaker/maskmaker.exe"))
MORPHRESTFILE = fixpath(os.path.abspath("./morphs/morph_rest.fbx"))


def str_is_float(x):
    try:
        a = float(x)
    except ValueError:
        return False
    else:
        return True


def str_is_int(x):
    try:
        a = int(x)
    except ValueError:
        return False
    else:
        return True


# ==============================================================================
# FIELD SEVERITIES
# ==============================================================================
def critical(field):
    return field in ["name", "author", "license", "copyright"]


def desired(field):
    return field in ["website"]


# ==============================================================================
# SYSTEM STUFF
# ==============================================================================

# Executes a shell command
# usage:
# 
# for line in execute(cmd):
#   print(line)
#
def execute(cmd):
    popen = subprocess.Popen(cmd, stdout=subprocess.PIPE, universal_newlines=True, shell=True)
    for stdout_line in iter(popen.stdout.readline, ""):
        yield stdout_line
    popen.stdout.close()
    return_code = popen.wait()
    if return_code:
        # raise subprocess.CalledProcessError(return_code, cmd)
        yield "ERROR " + cmd.split()[0] + " FAILED EXECUTION."


# Gets a list of files
def getFileList(folder):
    fileList = list()
    for root, subdir, files in os.walk(folder):
        for file in files:
            if file.lower().endswith(".fbx"):
                fileList.append(os.path.join(root, file).replace("\\", "/"))

    return fileList


def jsonFromFbx(fbxfile):
    return os.path.abspath(fbxfile).replace(".fbx", ".json").replace(".FBX", ".json")


def collapse_path(p):
    bits = p.replace("/", "\\").split("\\")
    for i in range(0, len(bits)):
        if i > 0:
            if bits[i] == "..":
                del bits[i]
                del bits[i - 1]
                break
    return "/".join(bits)


def make_path_relative(base, path):
    basebits = base.replace("/", "\\").split("\\")
    pathbits = path.replace("/", "\\").split("\\")
    while True and len(basebits) > 0 and len(pathbits) > 0:
        if basebits[0] == pathbits[0]:
            del basebits[0]
            del pathbits[0]
    return "\\".join(pathbits)


# ==============================================================================
# SVN
# ==============================================================================

def svnFileMissing():
    # run status and look for !
    cmd = SVNBIN + ' status -uq'
    for line in execute(cmd):
        if line.split()[0] in ["!"]:
            return True
    return False


def svnUpdate(outputWindow=None):
    # run update
    cmd = SVNBIN + ' update'
    arttoolUpdated = False
    for line in execute(cmd):
        if outputWindow:
            if "arttool" in line:
                arttoolUpdated = True
            outputWindow.append(line[:-1])
        else:
            print(line)
    return arttoolUpdated


def svnNeedsUpdate():
    # have
    cmd = SVNBIN + ' info'
    revHave = 0
    for line in execute(cmd):
        if "Last Changed Rev" in line:
            revHave = int(line.split()[-1])

    # head
    cmd = SVNBIN + ' info -r HEAD'
    revHead = 0
    for line in execute(cmd):
        if "Last Changed Rev" in line:
            revHead = int(line.split()[-1])

    return (revHead > revHave) or svnFileMissing()


def svnNeedsCommit():
    cmd = SVNBIN + ' status -uq'
    for line in execute(cmd):
        if line.split()[0] in ["A", "M", "D"]:
            return True
    return False


def svnGetFileStatus(filename):
    cmd = SVNBIN + ' status ' + os.path.abspath(filename)
    for line in execute(cmd):
        return line.split()[0]
    return ""


def svnIsFileNew(filename):
    return svnGetFileStatus(filename) == "?"


def svnAddFile(filename):
    if svnIsFileNew(filename):
        cmd = SVNBIN + " add " + os.path.abspath(filename)
        for line in execute(cmd):
            pass


# ==============================================================================
# MASKMAKER
# ==============================================================================
def maskmaker(command, kvpairs, files):
    cmd = MASKMAKERBIN + " " + command
    for k, v in kvpairs.items():
        if command == "tweak":
            cmd += ' "' + k + '=' + v + '"'
        if type(v) is str:
            cmd += " " + k + '="' + v + '"'
        else:
            cmd += " " + k + '=' + str(v)

    for f in files:
        cmd += " " + f

    print("----------")
    print(cmd)
    for line in execute(cmd):
        yield line[:-1]


def mmImport(fbxfile, metadata):
    CREATEKEYS = ["name", "uuid", "tier", "description", "author",
                  "tags", "category", "license", "website", "texture_max",
                  "intro_fade_time", "intro_duration"]
    d = dict()
    for k in CREATEKEYS:
        d[k] = metadata[k]

    jsonfile = jsonFromFbx(fbxfile)
    if metadata["is_morph"]:
        d["restfile"] = MORPHRESTFILE
        d["posefile"] = os.path.abspath(fbxfile)
        for line in maskmaker("morphimport", d, [jsonfile]):
            yield line
    else:
        d["file"] = os.path.abspath(fbxfile)
        for line in maskmaker("import", d, [jsonfile]):
            yield line


def mmDepends(fbxfile):
    cmd = MASKMAKERBIN + " depends " + '"' + os.path.abspath(fbxfile) + '"'
    deps = list()
    for line in execute(cmd):
        deps.append(line[:-1])
    return deps


# ==============================================================================
# META DATA
# ==============================================================================
def getMetaFolderName(fbxfile):
    dn = os.path.dirname(fbxfile)
    dn = os.path.join(dn, ".art")
    return fixpath(dn)


def createMetaFolder(folder, dosvn=False):
    if not os.path.exists(folder):
        os.makedirs(folder)
        if dosvn:
            svnAddFile(folder)


def getMetaFileName(fbxfile):
    metafolder = getMetaFolderName(fbxfile)
    metafile = os.path.join(metafolder, os.path.basename(fbxfile).lower().replace(".fbx", ".meta"))
    return fixpath(metafile)


def writeMetaData(metafile, metadata, dosvn=False):
    try:
        f = open(metafile, "w")
        f.write(json.dumps(metadata, indent=4))
        f.close()
    except:
        print("WRITING", metafile, "FAILED")
        print("WHAT THE HELL MAN")
    else:
        if dosvn:
            svnAddFile(metafile)


def createGetMetaData(fbxfile):
    metafolder = getMetaFolderName(fbxfile)
    createMetaFolder(metafolder, True)
    metafile = getMetaFileName(fbxfile)
    metadata = dict()
    if os.path.exists(metafile):
        # read existing metadata
        f = open(metafile, "r")
        contents = f.read()
        f.close()
        if len(contents) < 1:
            print("EMPTY META DATA FOUND", fbxfile)
        else:
            metadata = json.loads(contents)
            metadata["license"] = "Copyright 2017 - General Workings Inc. - All rights reserved."
            if "tier" not in metadata:
                metadata["tier"] = "1"
            if "is_intro" not in metadata:
                metadata["is_intro"] = False
            if "intro_fade_time" not in metadata:
                metadata["intro_fade_time"] = 10.0
            if "intro_duration" not in metadata:
                metadata["intro_duration"] = 2.13333
            if "release_with_plugin" not in metadata:
                metadata["release_with_plugin"] = False
    else:
        # create new metadata
        metadata["fbx"] = fbxfile
        metadata["name"] = ""
        metadata["description"] = ""
        metadata["author"] = ""
        metadata["tags"] = ""
        metadata["category"] = ""
        metadata["tier"] = "1"
        metadata["uuid"] = str(uuid.uuid4())
        metadata["depth_head"] = False
        metadata["is_morph"] = False
        metadata["is_vip"] = False
        metadata["is_intro"] = False
        metadata["texture_max"] = 256
        metadata["release_with_plugin"] = False
        metadata["do_not_release"] = False
        metadata["intro_fade_time"] = 10.0
        metadata["intro_duration"] = 2.13333
        metadata["license"] = "Copyright 2017 - General Workings Inc. - All rights reserved."
        metadata["website"] = "http://streamlabs.com"
        metadata["additions"] = list()
        # write it
        writeMetaData(metafile, metadata, True)

    return metadata


# checks status of meta data 
#
CHECKMETA_GOOD = 0
CHECKMETA_ERROR = 1
CHECKMETA_WARNING = 2
CHECKMETA_NORELEASE = 3
CHECKMETA_WITHPLUGIN = 4
MASK_UNKNOWN = -1
MASK_NORMAL = 0
MASK_MORPH = 1


def checkMetaData(metadata):
    masktype = MASK_UNKNOWN

    # is morph
    if metadata["is_morph"]:
        masktype = MASK_MORPH
    else:
        masktype = MASK_NORMAL

    # not for release
    if metadata["do_not_release"]:
        return CHECKMETA_NORELEASE, masktype
    elif "release_with_plugin" in metadata and metadata["release_with_plugin"]:
        return CHECKMETA_WITHPLUGIN, masktype

    # check for errors
    for field, value in metadata.items():
        if type(value) is str and critical(field) and len(value) == 0:
            return CHECKMETA_ERROR, masktype
        elif type(value) is str and desired(field) and len(value) == 0:
            return CHECKMETA_WARNING, masktype

    # check for icons
    fbxfile = metadata["fbx"]
    pf = os.path.abspath(fbxfile.lower().replace(".fbx", ".gif"))
    if not os.path.exists(pf):
        return CHECKMETA_WARNING, masktype

    # good
    return CHECKMETA_GOOD, masktype


def checkMetaDataFile(fbxfile):
    masktype = MASK_UNKNOWN
    metafile = getMetaFileName(fbxfile)
    if not os.path.exists(metafile):
        return CHECKMETA_ERROR, masktype

    # read existing metadata
    metadata = dict()
    f = open(metafile, "r")
    fc = f.read()
    f.close()

    if len(fc) > 0:
        metadata = json.loads(fc)
    else:
        print("EMPTY META DATA FILE", metafile)
        return CHECKMETA_ERROR, MASK_UNKNOWN

    # check it
    return checkMetaData(metadata)


# ==============================================================================
# CONFIG
# ==============================================================================

def createGetConfig():
    createMetaFolder("./.art")
    metafile = "./.art/config.meta"
    config = dict()
    if os.path.exists(metafile):
        f = open(metafile, "r")
        config = json.loads(f.read())
        f.close()
    else:
        config["x"] = 50
        config["y"] = 50
        writeMetaData(metafile, config)
    return config
