1. Download fastbuild
2. Put it on the path. (On windows: Computer->right click->properties->advanced system settings->Environment variables->PATH)
3. Go to fastbuild folder
4. Setup the hooks by moving the contents of git-hooks into .git/hook
5. Do git checkout (so it updates the visual studio files), you can also do it by calling fbuild vs2015-solution from the fastbuild folder.
6. Now you can just open the <root>/vs2017/Lua++.sln and work on Lua++.
