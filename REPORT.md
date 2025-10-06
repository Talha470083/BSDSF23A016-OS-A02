## 🧠 Questions & Solutions

---

### **1️⃣ What is the crucial difference between the `stat()` and `lstat()` system calls?**  
**In the context of the `ls` command, when is it more appropriate to use `lstat()`?**

#### ✅ **Answer:**
The **crucial difference** lies in how the two system calls handle **symbolic links**:

- **`stat()`**:  
  Follows symbolic links.  
  → If you call `stat()` on a symbolic link, it returns information about the **target file** that the link points to.

- **`lstat()`**:  
  Does **not** follow symbolic links.  
  → If you call `lstat()` on a symbolic link, it returns information about the **link itself**, not its target.

#### 💡 **In the context of the `ls` command:**
- When implementing `ls`, especially with the `-l` (long listing) option, it’s better to use **`lstat()`**.  
- This allows `ls` to display the symbolic link’s details (e.g., permissions, ownership, and link target) rather than those of the file it points to.  
- Example:  
  ```bash
  lrwxrwxrwx 1 user user 10 Oct  5 13:00 mylink -> target.txt
### 🧠 Question 2
**The `st_mode` field in `struct stat` is an integer that contains both the file type (e.g., regular
file, directory) and the permission bits. Explain how you can use bitwise operators (like `&`)
and predefined macros (like `S_IFDIR` or `S_IRUSR`) to extract this information.**

---

### 💡 Answer

The `st_mode` field of the `struct stat` structure stores both **file type** and **permission bits**.
You can use **bitwise operators** with **macros** from `<sys/stat.h>` to determine these properties.

- **File type bits** tell you whether it’s a file, directory, link, etc.
- **Permission bits** define read, write, and execute permissions.

You can use bitwise AND (`&`) to test these bits.

---

### 🧩 Example Explanation

To check if a file is a **directory**:
```c
if (st.st_mode & S_IFDIR)
    printf("This is a directory\n");

# Feature 3 — Column Display (ls-v1.2.0)

## General logic for "down then across" columnar printing
To print items in a "down then across" format we:
1. Gather all filenames into an array and determine the longest filename length.
2. Query the terminal width (via `ioctl(TIOCGWINSZ)`) and compute the column width as `max_filename_length + spacing`.
3. Compute the number of columns that fit: `cols = term_width / column_width` (minimum 1).
4. Compute rows needed: `rows = ceil(num_items / cols)`.
5. Print row by row. For row `r`, print items:
   - `names[r]`, `names[r + rows]`, `names[r + 2*rows]`, ..., until columns exhausted.
This layout fills columns top-to-bottom first, then left-to-right across columns. A single linear loop over filenames cannot produce this layout because the physical printed order is not the array order — it requires index arithmetic mapping rows and columns.

## Why a simple single loop is insufficient
A single loop iterating filenames in natural array order prints them left-to-right, top-to-bottom, producing a different visual layout (items appear across rows before moving down). The "down then across" pattern needs items grouped into `rows` slices per column, so each printed row must access non-contiguous elements from the filename array (element indices differ by `rows`). Hence we iterate rows and inside that iterate columns and compute `index = column * rows + row`.

## Purpose of `ioctl(TIOCGWINSZ)` in this context
`ioctl` with the `TIOCGWINSZ` request retrieves the current terminal window size (number of columns). This allows the program to compute how many columns of filenames fit in the user's terminal and dynamically adapt output to the user's environment. Without it, if we used a fixed width (such as 80 columns), the output may:
- Wrap incorrectly on narrow terminals, producing misaligned output.
- Underuse wide terminals (excessively narrow layout) and not present as many columns as possible.
Therefore runtime detection with `ioctl` produces the expected adaptive behavior similar to the standard `ls` utility.

## Limitations of fixed-width fallback
A fixed width (e.g., 80) can serve as a fallback if `ioctl` fails (e.g., not connected to a tty), but it reduces UX:
- On terminals narrower than 80, columns may overflow; long filenames could break layout.
- On very wide terminals, we won't leverage extra space to show more columns.
Thus, while acceptable as a fallback, it is inferior to detecting terminal size at runtime.

