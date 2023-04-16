# <img src="src/static/build/.tmp_update/res/icon_search.png?raw=true" width="24"> Search / <img src="src/static/build/.tmp_update/res/icon_filter.png?raw=true" width="24"> Filter for Onion

This app adds *Search* to the *Consoles* section and an optional *Filter* function for each console list.


## Search

- Go to the *Consoles* section and choose *Search*.
- Select `Enter search term...`.
- Choose OK or press <kbd>START</kbd> to initiate the search.
- The results can be viewed in a single list with all results or several lists grouped by console.
- Press <kbd>▲</kbd> (when on the first entry) to go to the search actions.
- Here you can clear the search or choose the `Search: [...]` option to edit the search term.

**Note:** When the roms are refreshed, the console lists will no longer be cached, and will not be included in the search. To include them, just visit the given console list and it will be scanned and cached.

> **Tip:** Instead of choosing "refresh roms" after adding a new rom, simply delete the `*_cache6.db` file inside the rom folder - now you'll only need to visit one console list! This is made even easier with the `Install filter` tool, which adds a `Refresh roms` shortcut to the bottom of each console list, which will only delete the corresponding cache.


### Tools

*Search* now has a *Tools* section which includes:

- **Fix favorites boxart** - Fixes the boxart of favorites added from search.
- **Sort favorites** - Quick access to sorting the favorites list alphabetically.
- **Add tools to favorites** - Adds shortcuts for the two favorites related tools to the favorites list.
- **Clean recent list** - Removes apps from recent list (bonus tool used internally to remove search related entries from the list).
- **Install filter** - Adds filter shortcuts to the bottom of each console list.
- **Uninstall filter** - Removes the filter shortcuts from each console list.


### Filter

To manually enable the filter option go to `Consoles` ➜ `Search` ➜ `Tools` ➜ `Install filter`. This will add two shortcuts to the bottom of each console list: `Filter` and `Refresh roms`. Use the `Filter` option to apply a filter to the list. The `Refresh roms` shortcut is a bonus feature, which allows you to refresh a single list, instead of the built-in counterpart which forces a refresh of all lists at once.

**Note:** Filters will not limit search results.


## Installation

Copy the contents of the [SearchFilter-v??.zip](https://github.com/Aemiii91/miyoomini-SearchFilter/releases) file to the root of your SD card. Then on your Miyoo Mini go to `Apps` and select `Install Search/Filter`, this will install or update Search/Filter.


## Screenshots

<img title="Consoles section" src="screenshots/001-v1.1.png?raw=true" width="49%"> <img title="Initial screen" src="screenshots/002-v1.1.png?raw=true" width="49%"> <img title="Tools" src="screenshots/003-v1.1.png?raw=true" width="49%"> <img title="Enter search term" src="screenshots/004-v1.1.png?raw=true" width="49%"> <img title="Results" src="screenshots/005-v1.1.png?raw=true" width="49%"> <img title="Edit or clear search" src="screenshots/006-v1.1.png?raw=true" width="49%">

---

<p align="right"><sup>Icons by <a href="https://icons8.com">Icons8</a></sup></p>
