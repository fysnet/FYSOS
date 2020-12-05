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

## Your contribution
If you wish to contribute to this project--and I hope you do--or wish to point out a bug, please contact me at: fys [at] fysnet [dot] net.
