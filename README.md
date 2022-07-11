# <img src="skeleton/App/SearchFilter/res/icon_search.png?raw=true" width="24"> Search / <img src="skeleton/App/SearchFilter/res/icon_filter.png?raw=true" width="24"> Filter for OnionOS

This app adds *Search* to the *Consoles* section of OnionOS and a *Filter* app under *Apps*.

## Search

<img title="Consoles section" src="screenshots/1.png?raw=true" width="150px"> <img title="Initial screen" src="screenshots/2.png?raw=true" width="150px"> <img title="Tools" src="screenshots/3.png?raw=true" width="150px"> <img title="Enter search term" src="screenshots/4.png?raw=true" width="150px"> <img title="Results" src="screenshots/5.png?raw=true" width="150px"> <img title="Edit or clear search" src="screenshots/6.png?raw=true" width="150px">

- Go to the *Consoles* section and choose *Search*.
- Press <kbd>A</kbd> again to enter a search term.
- Choose OK or press <kbd>START</kbd> to initiate the search.
- The results can be viewed in a single list with all results or several lists grouped by console.
- Press <kbd>▲</kbd> (when on the first entry) to go to the search actions.
- Here you can clear the search or choose the `Search: [...]` option to edit the search term.

### Notes

When the roms are refreshed, the console lists will no longer be cached, and will not be included in the search. To include them, just visit the given console list and it will be scanned and cached.

> **Tip:** Instead of choosing "refresh roms" after adding a new rom, simply delete the `*_cache2.db` file inside the rom folder. Now you'll only need to visit one console list.

### Tools

*Search* now has a *Tools* section which includes:

- **Fix favorites boxart** - Fixes the boxart of favorites added from search.
- **Sort favorites** - Quick access to sorting the favorites list alphabetically.
- **Add tools to favorites** - will add shortcuts to the two favorites tools to the favorites list.

## Filter

Applying a filter will hide all games in each console list that don't match the selected keywords.

- Open the *Filter* app and apply a filter keyword.
- The app applies the filter and tells you how many games was found for each console.
- Exit the app, and browse your ROM lists to see the filtered results.
- To restore the ROM lists, open the app and clear the filter by pressing X.

## Installation
Copy the contents of the [SearchFilter-v??.zip](https://github.com/Aemiii91/miyoomini-SearchFilter/releases) file to the root of your SD card.

### Updating

When updating, it is best to delete the contents of the `App/SearchFilter` directory on your SD card.
