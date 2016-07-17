set BUILD_DIR=..\build\emscripten-win32-debug
set TARGET_FILE=WeightedPerfectMatchingLib-debug.js

del ".\%TARGET_FILE%" /Q

md "%BUILD_DIR%"
call emcc "..\src\wpm\NonMatchingGraph.cpp" -o "%BUILD_DIR%\NonMatchingGraph.bc"
call emcc "..\src\wpm\BipartiteMatchingGraph.cpp" -o "%BUILD_DIR%\BipartiteMatchingGraph.bc"
call emcc "..\src\wpm\MatchingGraphConverter.cpp" -o "%BUILD_DIR%\MatchingGraphConverter.bc"
call emcc "..\src\wpm\PerfectMatchingFinder.cpp" -o "%BUILD_DIR%\PerfectMatchingFinder.bc"
call emcc "..\src\wpm\WeightedPerfectMatchingCLib.cpp" -o "%BUILD_DIR%\WeightedPerfectMatchingCLib.bc"
call emcc "%BUILD_DIR%\NonMatchingGraph.bc" "%BUILD_DIR%\BipartiteMatchingGraph.bc" "%BUILD_DIR%\MatchingGraphConverter.bc" "%BUILD_DIR%\PerfectMatchingFinder.bc" "%BUILD_DIR%\WeightedPerfectMatchingCLib.bc" -o "%BUILD_DIR%\%TARGET_FILE%" -s EXPORTED_FUNCTIONS="['_findBestPerfectMatching']" -s RESERVED_FUNCTION_POINTERS=1

copy "%BUILD_DIR%\%TARGET_FILE%" ".\%TARGET_FILE%"

pause
