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
from copy import deepcopy
from .additions import perform_addition
import getpass

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


def is_admin_user():
    if getpass.getuser() == "Ross":
        return True
    return False


# ==============================================================================
# AMAZON S3 SERVER
# ==============================================================================
S3_BUCKET = "facemasks-cdn.streamlabs.com"

def s3_upload(filename, key):
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
    s3.Bucket(S3_BUCKET).put_object(Key=key, Body=f, ACL='public-read', ContentType=cunt_type)



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
    basebits = base.lower().replace("/", "\\").split("\\")
    pathbits = path.lower().replace("/", "\\").split("\\")
    while len(basebits) > 0 and len(pathbits) > 0:
        if basebits[0] == pathbits[0]:
            del basebits[0]
            del pathbits[0]
        else:
            print("WTF MAN")
            print(basebits)
            print(pathbits)
            break
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

    return False

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
        cmd = SVNBIN + " add " + fixpath(filename)
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
        elif type(v) is str:
            cmd += " " + k + '="' + v + '"'
        else:
            cmd += " " + k + '=' + str(v)

    for f in files:
        cmd += " " + f

    print("---maskmaker-------")
    print(cmd)
    for line in execute(cmd):
        yield line[:-1]
    print(" ")


def mmGetCreateKeys(metadata):
    CREATEKEYS = ["name", "uuid", "tier", "description", "author",
                  "tags", "category", "license", "website", "texture_max",
                  "is_intro", "draw_video_with_mask", "intro_fade_time",
                  "intro_duration"]
    d = dict()
    for k in CREATEKEYS:
        d[k] = metadata[k]
    return d


def mmImport(fbxfile, metadatain):
    metadata = cleanMetadata(metadatain)
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


def mmMerge(jsonfile, metadatain):
    files = list()
    metadata = cleanMetadata(metadatain)
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
    metadata["tier"] = 1
    metadata["uuid"] = str(uuid.uuid4())
    metadata["depth_head"] = False
    metadata["is_morph"] = False
    metadata["is_vip"] = False
    metadata["is_intro"] = False
    metadata["draw_video_with_mask"] = False
    metadata["texture_max"] = 256
    metadata["release_with_plugin"] = False
    metadata["do_not_release"] = False
    metadata["intro_fade_time"] = 0.333333
    metadata["intro_duration"] = 2.13333
    metadata["license"] = "Copyright 2017 - General Workings Inc. - All rights reserved."
    metadata["website"] = "http://streamlabs.com"
    metadata["additions"] = list()
    return metadata


def cleanMetadata(metadata):
    dd = deepcopy(metadata)
    if "tags" in dd and dd["tags"]:
        dd["tags"] = dd["tags"].lower().replace(", ", ",")
    else:
        dd["tags"] = ""
    dd["author"] = dd["author"].replace(", ", ",")
    return dd


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


def fixMissingMetaData(metadata):
    # Fix missing/incorrect fields
    #
    if "General Working Inc" in metadata["license"]:
        metadata["license"] = "Copyright 2017 - General Workings Inc. - All rights reserved."
    if "tier" not in metadata:
        metadata["tier"] = 1
    if "is_intro" not in metadata:
        metadata["is_intro"] = False
    if "draw_video_with_mask" not in metadata:
        metadata["draw_video_with_mask"] = False
    if "intro_fade_time" not in metadata:
        metadata["intro_fade_time"] = 0.3333
    if "intro_duration" not in metadata:
        metadata["intro_duration"] = 2.13333
    if "release_with_plugin" not in metadata:
        metadata["release_with_plugin"] = False


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
            fixMissingMetaData(metadata)

    else:
        # make new metadata and write it
        metadata = newMetaData(fbxfile)
        writeMetaData(metafile, metadata, True)

    if metadata["fbx"].lower().endswith(".json"):
        metadata["category"] = "Combo"
        if len(metadata["additions"]) == 0:
            for i in range(0, 10):
                metadata["additions"].append("")

    return deepcopy(metadata)


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


def critical_mask(field):
    return field in ["name", "author", "license", "description", "copyright"]

def critical_combo(field):
    return field in ["name", "license", "description", "copyright"]


def desired(field):
    return field in ["website"]


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

    # combo?
    critical = critical_mask
    if metadata["fbx"].lower().endswith(".json"):
        critical = critical_combo

    # check for errors
    for field, value in metadata.items():
        if type(value) is str and critical(field) and len(value) == 0:
            #print(metadata["fbx"], " missing ", field)
            return CHECKMETA_ERROR, masktype
        elif type(value) is str and desired(field) and len(value) == 0:
            #print(metadata["fbx"], " missing ", field)
            return CHECKMETA_WARNING, masktype

    # check for icons
    fbxfile = metadata["fbx"]
    pf = os.path.abspath(fbxfile.lower().replace(".fbx", ".gif").replace(".json", ".gif"))
    if not os.path.exists(pf):
        #print(fbxfile, " missing ", pf)
        return CHECKMETA_WARNING, masktype
    pf = os.path.abspath(fbxfile.lower().replace(".fbx", ".png").replace(".json", ".gif"))
    if not os.path.exists(pf):
        #print(fbxfile, " missing ", pf)
        return CHECKMETA_WARNING, masktype
    pf = os.path.abspath(fbxfile.lower().replace(".fbx", ".mp4").replace(".json", ".gif"))
    if not os.path.exists(pf):
        #print(fbxfile, " missing ", pf)
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


# No error checking
def loadMetadataFile(fbxfile):
    metafile = getMetaFileName(fbxfile)
    if not os.path.exists(metafile):
        return None

    # read metafile
    f = open(metafile, "r")
    fc = f.read()
    f.close()

    metadata = None
    if len(fc) > 0:
        try:
            metadata = json.loads(fc)
        except:
            metadata = None

    fixMissingMetaData(metadata)

    return metadata



# Combine author & tags for a combo
def getCombinedComboMeta(metadata):
    author = list()
    tags = list()
    if metadata["fbx"].lower().endswith(".json"):
        for f in metadata["additions"]:
            if len(f) > 0:
                md = loadMetadataFile(f)
                tt = md["tags"].split(",")
                for ttt in tt:
                    ttt = ttt.strip()
                    if ttt not in tags:
                        tags.append(ttt)
                tt = md["author"].split(",")
                for ttt in tt:
                    ttt = ttt.strip()
                    if ttt not in author:
                        author.append(ttt)
    d = dict()
    d["tags"] = ",".join(tags)
    if len(author) > 1:
        d["author"] = ",".join(author)
    elif len(author) == 1:
        d["author"] = author[0]
    else:
        d["author"] = "Streamlabs"
    return d

# ==============================================================================
# DEPENDENCIES
# ==============================================================================

def doesFileNeedRebuilding(filename, metadata=None):
    if metadata is None:
        metadata = loadMetadataFile(filename)
    metafile = getMetaFileName(filename)
    if filename.lower().endswith(".fbx"):
        jsonfile = filename.lower().replace(".fbx",".json")
        fbxmodtime = os.path.getmtime(filename)
        # missing json
        if not os.path.exists(jsonfile):
            return True
        jsonmodtime = os.path.getmtime(jsonfile)
        # fbx modtime
        if fbxmodtime > jsonmodtime:
            return True
        # missing meta
        if not os.path.exists(metafile):
            return True
        # meta modtime
        if os.path.getmtime(metafile) > jsonmodtime:
            return True
        # dependencies
        for dep in metadata["dependencies"]:
            f = os.path.abspath(dep["file"])
            if os.path.exists(f):
                if os.path.getmtime(f) > jsonmodtime:
                    return True
            else:
                return True

    else: # .json (combo)
        # missing json
        if not os.path.exists(filename):
            return True
        jsonmodtime = os.path.getmtime(filename)
        # meta modtime
        if os.path.getmtime(metafile) > jsonmodtime:
            return True
        # dependencies
        for dep in metadata["dependencies"]:
            f = os.path.abspath(dep["file"])
            if os.path.exists(f):
                if os.path.getmtime(f) > jsonmodtime:
                    return True
            else:
                return True

    return False


def getComboDependencies(metadata):
    # save mod times of dependent jsons
    combodeps = list()
    missing = list()
    for fbxfile in metadata["additions"]:
        if len(fbxfile) > 0:
            f = fbxfile.lower().replace(".fbx", ".json")
            if os.path.exists(os.path.abspath(f)):
                combodeps.append({"file": f, "modtime": os.path.getmtime(f)})
            else:
                combodeps.append({"file": f, "modtime": 0})
                missing.append(f)
    return combodeps,missing

def getMaskDependencies(metadata):
    fbxfile = metadata["fbx"]

    # save mod times of dependent pngs
    deps = mmDepends(fbxfile)
    dirname = os.path.dirname(fbxfile)
    metadeps = list()
    missing = list()
    for d in deps:
        f = collapse_path(os.path.join(dirname, d))
        if os.path.exists(os.path.abspath(f)):
            metadeps.append({"file": f, "modtime": os.path.getmtime(f)})
        else:
            ff = os.path.join(os.path.dirname(fbxfile), os.path.basename(f)).replace("\\", "/")
            if os.path.exists(ff):
                metadeps.append({"file": ff, "modtime": os.path.getmtime(ff)})
            else:
                metadeps.append({"file": f, "modtime": 0})
                missing.append(f)
    return metadeps,missing

def getDependencies(metadata):
    if metadata["fbx"].lower().endswith(".fbx"):
        return getMaskDependencies(metadata)
    else:
        return getComboDependencies(metadata)


# ==============================================================================
# BUILD
# ==============================================================================


def buildCombo(combofile, outputWindow, metadata=None):

    if metadata is None:
        metadata = loadMetadataFile(combofile)

    # save dependencies
    deps, missing = getDependencies(metadata)
    metadata["dependencies"] = deps
    metafile = getMetaFileName(combofile)
    writeMetaData(metafile, metadata, True)

    # run maskmaker merge, add json to svn
    for line in mmMerge(combofile, metadata):
        outputWindow.append(line)
    svnAddFile(combofile)

    return deps,missing


def buildMask(fbxfile, outputWindow, metadata=None):

    print("buildMask", fbxfile)

    if metadata is None:
        metadata = loadMetadataFile(fbxfile)
    if metadata is None:
        return None, None

    # save dependencies
    deps, missing = getDependencies(metadata)
    metadata["dependencies"] = deps
    metafile = getMetaFileName(fbxfile)
    writeMetaData(metafile, metadata, True)

    # import fbx to json
    for line in mmImport(fbxfile, metadata):
        outputWindow.append(line)

    # add json to svn
    jsonfile = jsonFromFbx(fbxfile)
    svnAddFile(jsonfile)

    # add depth head
    if metadata["depth_head"]:
        # material
        kvp = {"type": "material",
               "name": "depth_head_mat",
               "effect": "effectDefault",
               "depth-only": True}
        for line in maskmaker("addres", kvp, [jsonfile]):
            outputWindow.append(line)
        # model
        kvp = {"type": "model",
               "name": "depth_head_mdl",
               "mesh": "meshHead",
               "material": "depth_head_mat"}
        for line in maskmaker("addres", kvp, [jsonfile]):
            outputWindow.append(line)
        # part
        kvp = {"type": "model",
               "name": "depth_head",
               "resource": "depth_head_mdl"}
        for line in maskmaker("addpart", kvp, [jsonfile]):
            outputWindow.append(line)

    # additions
    if "additions" in metadata:
        for addn in metadata["additions"]:
            perform_addition(addn, jsonfile, outputWindow)

    return deps,missing



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

def getConfigFolder():
    return fixpath(os.path.join(os.path.expanduser("~"), ".art"))


def getConfigFile():
    fldr = getConfigFolder()
    return os.path.join(fldr, "config.json")


def createGetConfig():
    try:
        fldr = getConfigFolder()
        createMetaFolder(fldr)
        metafile = getConfigFile()
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
