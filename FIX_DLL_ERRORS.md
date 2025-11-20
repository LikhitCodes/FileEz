# Fixing "DLL Not Found" Errors

## Problem
When running `p2p_share.exe` on another computer, you get errors like:
```
libstdc++-6.dll not found
libgcc_s_dw2-1.dll not found
```

## Why This Happens
The program was compiled with MinGW, which requires certain runtime DLLs. Your computer has them (because MinGW is installed), but your friend's computer doesn't.

---

## Solution 1: Bundle DLLs (Recommended)

### Step 1: Copy Required DLLs

**Automatic Method:**
```bash
FIX_MISSING_DLLS.bat
```

**Manual Method:**
1. Find your MinGW installation (usually `C:\MinGW\bin\`)
2. Copy these files to your project folder:
   - `libstdc++-6.dll`
   - `libgcc_s_dw2-1.dll` (or `libgcc_s_seh-1.dll`)
   - `libwinpthread-1.dll` (if exists)

### Step 2: Create Package with DLLs
```bash
CREATE_PORTABLE_PACKAGE.bat
```

This will now include the DLLs in the package!

### Step 3: Send to Friend
The ZIP file will now work on any Windows computer!

---

## Solution 2: Static Linking (Alternative)

Compile the program to include all libraries statically (no DLLs needed).

### Update Makefile:

Add `-static` flag to LDFLAGS:

```makefile
# Find this line in Makefile:
LDFLAGS = -lws2_32

# Change to:
LDFLAGS = -static -lws2_32 -static-libgcc -static-libstdc++
```

### Recompile:
```bash
mingw32-make clean
mingw32-make
```

### Result:
- Larger EXE file (~2-3 MB instead of 200 KB)
- But NO DLLs needed!
- Works on any Windows computer

---

## Solution 3: Install MinGW on Friend's Computer

**Not Recommended** (defeats the purpose of portable app)

Your friend can install MinGW:
1. Download from: https://sourceforge.net/projects/mingw/
2. Install MinGW
3. Add `C:\MinGW\bin` to PATH
4. Run your program

---

## Quick Fix Guide

### For You (Developer):

**Option A - Bundle DLLs (Easiest):**
```bash
1. FIX_MISSING_DLLS.bat
2. CREATE_PORTABLE_PACKAGE.bat
3. Send new ZIP to friend
```

**Option B - Static Linking (Best):**
```bash
1. Edit Makefile, add: -static -static-libgcc -static-libstdc++
2. mingw32-make clean
3. mingw32-make
4. CREATE_PORTABLE_PACKAGE.bat
5. Send new ZIP to friend
```

### For Your Friend (User):

**Temporary Fix:**
Download the DLLs and put them in same folder as `p2p_share.exe`:
- https://github.com/mstorsjo/llvm-mingw/releases (look for runtime DLLs)

Or ask you to send a new package with DLLs included!

---

## Detailed: Static Linking Setup

### 1. Open Makefile in text editor

### 2. Find the LDFLAGS line:
```makefile
# Platform-specific settings
ifeq ($(OS),Windows_NT)
    LDFLAGS = -lws2_32
```

### 3. Change to:
```makefile
# Platform-specific settings
ifeq ($(OS),Windows_NT)
    LDFLAGS = -static -lws2_32 -static-libgcc -static-libstdc++
```

### 4. Recompile:
```bash
mingw32-make clean
mingw32-make
```

### 5. Test:
```bash
# Check file size (should be larger now)
dir p2p_share.exe

# Should be ~2-3 MB instead of ~200 KB
```

### 6. Create package:
```bash
CREATE_PORTABLE_PACKAGE.bat
```

### 7. This package will work EVERYWHERE!

---

## Comparison of Solutions

| Solution | Pros | Cons |
|----------|------|------|
| **Bundle DLLs** | Easy, small EXE | Need to include DLLs |
| **Static Linking** | Single EXE, no DLLs | Larger file size |
| **Install MinGW** | Nothing to change | Not portable |

**Recommendation:** Use **Static Linking** for best portability!

---

## Verification

### Check if DLLs are needed:

**Windows:**
```bash
# Use Dependency Walker or:
dumpbin /dependents p2p_share.exe
```

**After static linking, you should only see:**
- KERNEL32.dll (Windows system DLL - always present)
- WS2_32.dll (Windows sockets - always present)
- No MinGW DLLs!

---

## Common DLL Files Needed

If bundling DLLs, you need:

### Required:
- `libstdc++-6.dll` - C++ standard library
- `libgcc_s_dw2-1.dll` - GCC runtime (32-bit DWARF)
  OR
- `libgcc_s_seh-1.dll` - GCC runtime (64-bit SEH)

### Optional:
- `libwinpthread-1.dll` - Threading support (if using threads)

### Where to find them:
```
C:\MinGW\bin\
C:\msys64\mingw64\bin\
C:\mingw32\bin\
```

---

## Testing Your Package

### Before sending to friend:

1. **Test on clean VM or another computer**
2. **Or temporarily rename MinGW folder:**
   ```bash
   ren C:\MinGW C:\MinGW_backup
   # Test your program
   ren C:\MinGW_backup C:\MinGW
   ```

3. **If it works without MinGW folder, you're good!**

---

## Quick Reference

### I want the easiest solution:
```bash
1. FIX_MISSING_DLLS.bat
2. CREATE_PORTABLE_PACKAGE.bat
3. Done!
```

### I want the best solution (no DLLs):
```bash
1. Edit Makefile: Add -static -static-libgcc -static-libstdc++
2. mingw32-make clean
3. mingw32-make
4. CREATE_PORTABLE_PACKAGE.bat
5. Done!
```

### My friend needs it NOW:
```bash
1. Copy these files to same folder as p2p_share.exe:
   - libstdc++-6.dll
   - libgcc_s_dw2-1.dll
2. Send all files together
3. Done!
```

---

## Summary

✅ **Best Solution:** Static linking (edit Makefile)
✅ **Quick Solution:** Bundle DLLs (run FIX_MISSING_DLLS.bat)
✅ **Emergency Solution:** Send DLLs separately

**After fixing, your program will work on ANY Windows computer!** 🚀
