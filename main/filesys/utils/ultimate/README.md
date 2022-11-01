## Utimate: The ultimate image browser

This is the source code for the [Ultimate](http://www.fysnet.net/ultimate/index.htm) utility.

<img src=https://www.fysnet.net/ultimate/demodisk64.png>

## Please note:
<pre>
 * THIS SOFTWARE, DOCUMENTATION, BINARY FILES, OR OTHER ITEM, HEREBY FURTHER KNOWN
 * AS 'PRODUCT', IS  PROVIDED BY THE COPYRIGHT  HOLDER AND CONTRIBUTOR "AS IS" AND
 * ANY EXPRESS OR IMPLIED  WARRANTIES, INCLUDING, BUT NOT  LIMITED TO, THE IMPLIED
 * WARRANTIES  OF  MERCHANTABILITY  AND  FITNESS  FOR  A  PARTICULAR  PURPOSE  ARE 
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT  OWNER OR CONTRIBUTOR BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,  OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO,  PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER  CAUSED AND ON
 * ANY  THEORY OF  LIABILITY, WHETHER  IN  CONTRACT,  STRICT  LIABILITY,  OR  TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN  ANY WAY  OUT OF THE USE OF THIS
 * PRODUCT, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  READER AND/OR USER
 * USES AS THEIR OWN RISK.'
</pre>

## Home page
The home page for this utility is at [http://www.fysnet.net/ultimate/index.htm](http://www.fysnet.net/ultimate/index.htm)<br />
There you will see examples and help files for this utility.

## Things I wish to add/change:
- [ ] Currently the code uses a "static" memory allocation scheme to allocate each partition.
This uses up space in the executable as well as limiting the count of partitions it will allow.  I would like to change this to allocate memory instead.  This will free up .EXE size, as well as remove the limit of partitions it will allow.
- [ ] There are items within each partition "type" that I need to fix or add to.
- [ ] The current code release is for both 32- and 64-bit Windows with no code modification.  However, for the "About" dialog to display the correct version, you must modify the description for 32-bit. See [Line 81 in ultimate.rc](https://github.com/fysnet/FYSOS/blob/master/main/filesys/utils/ultimate/ultimate.rc#L81).

## To Build
- [X] 64-bit Windows: Using Microsoft Visual Studio (2019), create an empty workspace.  Add all of the files in this folder and then the 'res' folder.  Add the _WIN64 definition to your resource settings. Rebuild Solution.
- [X] 32-bit Windows: Using Microsoft Visual C++ 6.0, create an empty workspace.  Add all of the files in this folder and then the 'res' folder.  Rebuild All. Three might be a few other small changes to make it compile on 32-bit MSVC++ 6.0, after all MSVC++ 6.0 is copyrighted 1994-98 :-)
- [ ] I have not built with any other compiler.  I am guessing that someone could build using GCC for Windows, and possibly GCC and mingw (?) for Linux?  However, I do not use Linux or GCC (for Windows), and have no idea if it is even possible.

## Your contribution
If you wish to contribute to this project--and I hope you do--or wish to point out a bug, please contact me at: fys [at] fysnet [dot] net.

## Other notes
Most likely I will update the executables more frequently than the source code.  Therefore, the source may lag behind.  The release folder will hold the most recent executables, as I fix bugs and add new features.  The source code will be updated only on major releases.  However, if you need the latest source, contact me and I will see about updating it.

There are known bugs and other issues.  I will see if I can figure these out and fix them.  However, I do this as a hobby, and interest specifies when I work on it.

Please note that the code may look like it was quickly put together. In fact it was. I use this app to test my driver code and modify existing image files.  Nothing more.  If I need a quick addition, I will add it and then move on.  My main focus is not this app.  My main focus is the image files I modify with it.  Therefore, the code may and probably does look ugly. :-)  I don't really care as long as it does what I need it to at the moment.  It is, in all intents and purposes, a tool that I continue to improve and will probably be a never ending process.

Thank you all for your support.
