set BUILD_DIR=..\build\emscripten-win32-release
set TARGET_FILE=WeightedPerfectMatchingLib-release.js
set EMCC_NATIVE_OPTIMIZER=0

del .\%TARGET_FILE% /Q

md "%BUILD_DIR%"
call emcc -O2 "..\src\wpm\NonMatchingGraph.cpp" -o "%BUILD_DIR%\NonMatchingGraph.bc"
call emcc -O2 "..\src\wpm\BipartiteMatchingGraph.cpp" -o "%BUILD_DIR%\BipartiteMatchingGraph.bc"
call emcc -O2 "..\src\wpm\MatchingGraphConverter.cpp" -o "%BUILD_DIR%\MatchingGraphConverter.bc"
call emcc -O2 "..\src\wpm\PerfectMatchingFinder.cpp" -o "%BUILD_DIR%\PerfectMatchingFinder.bc"
call emcc -O2 "..\src\wpm\WeightedPerfectMatchingCLib.cpp" -o "%BUILD_DIR%\WeightedPerfectMatchingCLib.bc"
call emcc -O2 "%BUILD_DIR%\NonMatchingGraph.bc" "%BUILD_DIR%\BipartiteMatchingGraph.bc" "%BUILD_DIR%\MatchingGraphConverter.bc" "%BUILD_DIR%\PerfectMatchingFinder.bc" "%BUILD_DIR%\WeightedPerfectMatchingCLib.bc" -o "%BUILD_DIR%\%TARGET_FILE%" -s EXPORTED_FUNCTIONS="['_findBestPerfectMatching']" -s RESERVED_FUNCTION_POINTERS=1

copy "%BUILD_DIR%\%TARGET_FILE%" ".\%TARGET_FILE%"
copy "%BUILD_DIR%\%TARGET_FILE%.mem" ".\%TARGET_FILE%.mem"

pause
