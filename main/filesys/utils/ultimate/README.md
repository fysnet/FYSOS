## Utimate: The ultimate image browser

This is the source code for the [Ultimate](http://www.fysnet.net/ultimate/index.htm) utility.

<img src=http://www.fysnet.net/ultimate/demodisk.png>

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
This uses up space in the executable as well as limiting the count of partitions it will
allow.  I would like to change this to allocate memory instead.  This will free up .EXE
size, as well as remove the limit of partitions it will allow.
- [ ] There are items within each partition "type" that I need to fix or add to.
- [ ] The current code release is for 64-bit Windows.  I few (slight) modifications must be
made if you wish to compile for 32-bit Windows.

## To Build
- [X] 64-bit Windows: Using Microsoft Visual Studio (2019), create an empty workspace.  Add all of the files in this folder and then the 'res' folder.  Rebuild Solution. (I am sure you will need to tweak some settings: 'Release:x64', etc.)
- [X] 32-bit Windows: Using Microsoft Visual C++ 6.0, create an empty workspace.  Add all of the files in this folder and then the 'res' folder.  Rebuild All.  Note that a few items might need to be modified.  For example, [Line 81 in ultimate.rc](https://github.com/fysnet/FYSOS/blob/master/main/filesys/utils/ultimate/ultimate.rc#L81) currently states a 64-bit version.  I believe there were a few other small changes I had to do to make it compile on 32-bit MSVC++ 6.0, after all MSVC++ 6.0 is copyrighted 1994-98 :-)
- [ ] I have not built with any other compiler.  I am guessing that someone could build using GCC for Windows, and possibly GCC and mingw (?) for Linux?  However, I do not use Linux or GCC (for Windows), and have no idea if it is even possible.

## Your contribution
If you wish to contribute to this project--and I hope you do--or wish to point out a bug, please contact me at: fys [at] fysnet [dot] net.
