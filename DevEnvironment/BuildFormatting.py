
SECTION_LENGTH = 60
DEPTH_COLOR_TABLE = [
    35,32,36,34,33
]

gSectionDepth=0

def PrintBar(label: str, barChar : str, length : int):
    global gSectionDepth

    nameLength = len(label)
    divCharactersToPrint = length - nameLength - 2
    leftCharsToPrint = round(divCharactersToPrint / 2)
    rightCharsToPrint = divCharactersToPrint - leftCharsToPrint
    while leftCharsToPrint:
        print(barChar, end ="")
        leftCharsToPrint-=1
    print(" ", end ="")
    print(label, end ="")
    print(" ", end ="")
    while rightCharsToPrint:
        print(barChar, end ="")
        rightCharsToPrint-=1

def BeginSection(sectionName : str):
    global gSectionDepth

    color = str(DEPTH_COLOR_TABLE[gSectionDepth % len(DEPTH_COLOR_TABLE)])
    print("\x1b[1;" + color + "m", end ="")
    PrintBar(sectionName, "#", SECTION_LENGTH)
    print(" AFEX\033[0m")
    gSectionDepth+=1

def EndSection(sectionName : str):
    global gSectionDepth
    global SECTION_LENGTH
    global DEPTH_COLOR_TABLE

    gSectionDepth-=1
    color = str(DEPTH_COLOR_TABLE[gSectionDepth % len(DEPTH_COLOR_TABLE)])
    print("\x1b[0;" + color + "m", end ="")
    PrintBar("/" + sectionName, "=", SECTION_LENGTH)
    print(" AFEX\033[0m")

def PrintContext(context : str):
    global gSectionDepth
    global DEPTH_COLOR_TABLE
    color = str(DEPTH_COLOR_TABLE[(gSectionDepth-1) % (len(DEPTH_COLOR_TABLE))])
    print("\x1b[1;" + color + "m", end ="")
    for line in context.splitlines():
        print(">>  " + line)
    print("\033[0m", end ="")

def PrintError(error : str, doExcept : bool = False):
    print("\x1b[3;31;40m", end="")
    print(error)
    print("\033[0m", end="")
    if doExcept:
        raise Exception(error)
