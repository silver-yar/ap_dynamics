include(FetchContent)

FetchContent_Declare(
        catch2
        GIT_REPOSITORY "https://github.com/catchorg/Catch2.git"
        GIT_TAG 531a149ae70dd4143ba1ac28e33d40ab612d3897 # v2.13.6
)
add_subdirectory(catch2)

FetchContent_Declare(
        juce
        GIT_REPOSITORY "https://github.com/juce-framework/JUCE.git"
        GIT_TAG 6.1.6
)
add_subdirectory(juce)