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
import sys, subprocess, os, json, uuid, boto3


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

# Make sure we are in the right place. Clyde has issues.
CLYDE_HOME = "F:\\\\Work\\StreamLabs\\SLOBS\\SLART"
if os.path.exists(CLYDE_HOME):
    os.chdir(CLYDE_HOME)
JAKE_HOME = "G:\\\\STREAMLABS\\slart"
if os.path.exists(JAKE_HOME):
    os.chdir(JAKE_HOME)
ROSS_HOME = "C:\\\\STREAMLABS\\slart"
if os.path.exists(ROSS_HOME):
    os.chdir(ROSS_HOME)

SVNBIN = os.path.abspath(os.path.join("c:\\", '"Program Files"', "TortoiseSVN", "bin", "svn.exe"))
MASKMAKERBIN = fixpath(os.path.abspath("./maskmaker/maskmaker.exe"))
MORPHRESTFILE = fixpath(os.path.abspath("./morphs/morph_rest.fbx"))



# ==============================================================================
# AMAZON S3 SERVER
# ==============================================================================
S3_BUCKET = "facemasks-cdn.streamlabs.com"

def s3_upload(filename):
    f = open(fixpath(os.path.abspath(filename)), "rb")
    s3 = boto3.resource("s3")
    cunt_type = "application/octet-stream"
    if filename.endswith(".gif"):
        cunt_type = "image/gif"
    elif filename.endswith(".png"):
        cunt_type = "image/png"
    elif filename.endswith(".mp4"):
        cunt_type = "video/mp4"
    elif filename.endswith(".json"):
        cunt_type = "application/json"
    s3.Bucket(S3_BUCKET).put_object(Key=os.path.basename(filename), Body=f, ACL='public-read', ContentType=cunt_type)

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


import os
for root,subdir,files in os.walk("."):
    for file in files:
        f = os.path.abspath(os.path.join(root, file))
        if f.lower().endswith(".json"):
            if not os.path.exists(f.lower().replace(".json",".fbx")):
                print(os.path.join(root,file))


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


# Gets a list of fbx files
def getFbxFileList(folder):
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
    pathbits.insert(0,".")
    return "/".join(pathbits)


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


def mmGetCreateKeys(metadata):
    CREATEKEYS = ["name", "uuid", "tier", "description", "author",
                  "tags", "category", "license", "website", "texture_max",
                  "is_intro", "intro_fade_time", "intro_duration"]
    d = dict()
    for k in CREATEKEYS:
        d[k] = metadata[k]
    return d

def mmImport(fbxfile, metadata):
    d = mmGetCreateKeys(metadata)

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

def mmMerge(jsonfile, metadata):
    files = list()
    for f in metadata["additions"]:
        if len(f) > 0:
            files.append(fixpath(os.path.abspath(f.lower().replace(".fbx",".json"))))
    files.append(fixpath(os.path.abspath(jsonfile)))
    d = mmGetCreateKeys(metadata)
    for line in maskmaker("merge", d, files):
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

def newMetaData(fbxfile):
    # create new metadata
    metadata = dict()
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
    metadata["intro_fade_time"] = 0.333333
    metadata["intro_duration"] = 2.13333
    metadata["license"] = "Copyright 2017 - General Workings Inc. - All rights reserved."
    metadata["website"] = "http://streamlabs.com"
    metadata["additions"] = list()
    return metadata


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
    fn = os.path.basename(fbxfile).lower().replace(".fbx", ".meta").replace(".json", ".combo")
    metafile = os.path.join(metafolder, fn)
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
            print("EMPTY META FILE:", metafile)
            # make new metadata and write it
            metadata = newMetaData(fbxfile)
            writeMetaData(metafile, metadata, True)

        else:
            metadata = json.loads(contents)
            metadata["license"] = "Copyright 2017 - General Workings Inc. - All rights reserved."
            if "tier" not in metadata:
                metadata["tier"] = "1"
            if "is_intro" not in metadata:
                metadata["is_intro"] = False
            if "intro_fade_time" not in metadata:
                metadata["intro_fade_time"] = 0.3333
            if "intro_duration" not in metadata:
                metadata["intro_duration"] = 2.13333
            if "release_with_plugin" not in metadata:
                metadata["release_with_plugin"] = False
    else:
        # make new metadata and write it
        metadata = newMetaData(fbxfile)
        writeMetaData(metafile, metadata, True)

    if metadata["fbx"].endswith(".json"):
        metadata["category"] = "Combo"
        if len(metadata["additions"]) == 0:
            for i in range(0, 10):
                metadata["additions"].append("")

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
    if "is_morph" not in metadata:
        return CHECKMETA_ERROR, MASK_UNKNOWN

    masktype = MASK_UNKNOWN

    # is morph
    if metadata["is_morph"]:
        masktype = MASK_MORPH
    else:
        masktype = MASK_NORMAL

    # not for release
    if metadata["do_not_release"]:
        return CHECKMETA_NORELEASE, masktype

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

    if "release_with_plugin" in metadata and metadata["release_with_plugin"]:
        return CHECKMETA_WITHPLUGIN, masktype

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
        try:
            metadata = json.loads(fc)
        except:
            print("FUCKED UP META DATA!", metafile)
            return CHECKMETA_ERROR, MASK_UNKNOWN
    else:
        print("EMPTY META DATA FILE", metafile)
        return CHECKMETA_ERROR, MASK_UNKNOWN

    # check it
    return checkMetaData(metadata)




# ==============================================================================
# COMBOS
# ==============================================================================

# Gets a list of combos
# - entries are the json files
#
def getComboFileList(folder):
    fileList = list()
    for root, subdir, files in os.walk(folder):
        for file in files:
            if file.lower().endswith(".combo"):
                f = os.path.join(root, file).replace("\\","/").replace("/.art/","/").replace(".combo",".json")
                fileList.append(f)

    return fileList




# ==============================================================================
# CONFIG
# ==============================================================================

def createGetConfig():
    try:
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
    except:
        config = dict()
        config["x"] = 50
        config["y"] = 50

    return config
