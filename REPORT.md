## 📘 LS Assignment — Conceptual Questions and Answers

### 1️⃣ What is the crucial difference between the `stat()` and `lstat()` system calls? In the context of the `ls` command, when is it more appropriate to use `lstat()`?
**Answer:**  
The key difference is that `stat()` follows symbolic links, while `lstat()` does not.  
- `stat()` retrieves information about the **target file** the link points to.  
- `lstat()` retrieves information about the **link itself**.  
📌 In the context of the `ls` command, `lstat()` is more appropriate because `ls` needs to display properties of the symbolic link itself (like its name and target), not just the file it points to.

---

### 2️⃣ The `st_mode` field in `struct stat` contains both the file type and permission bits. Explain how you can use bitwise operators (like `&`) and predefined macros (like `S_IFDIR` or `S_IRUSR`) to extract this information.
**Answer:**  
You can use bitwise AND (`&`) along with macros to check specific bits in `st_mode`.  
Example:
```c
if (st.st_mode & S_IFDIR)   // Check if it's a directory
    printf("Directory\n");

if (st.st_mode & S_IRUSR)   // Check if owner can read
    printf("Readable by owner\n");
```
Each macro corresponds to a specific bitmask defined in `<sys/stat.h>` for file types or permissions.

---

### 3️⃣ Explain the general logic for printing items in a "down then across" columnar format. Why is a simple single loop through the list of filenames insufficient for this task?
**Answer:**  
In “down then across” format, filenames fill vertically first, then move horizontally across columns.  
You must:
- Calculate the **number of rows and columns** based on terminal width and longest filename.
- Use **two nested loops** or index math to print vertically by column.

A single loop is insufficient because it prints filenames linearly (left to right), whereas “down then across” requires multi-dimensional indexing to simulate column-based layout.

---

### 4️⃣ What is the purpose of the `ioctl` system call in this context? What would be the limitations of your program if you only used a fixed-width fallback (e.g., 80 columns) instead of detecting the terminal size?
**Answer:**  
`ioctl()` with `TIOCGWINSZ` retrieves the terminal window size dynamically.  
This allows your program to adapt column width and layout to fit the user’s screen.  
If you used a fixed width (like 80 columns), output could:
- Appear misaligned on wider or narrower terminals.
- Truncate filenames.
- Fail to use available space efficiently.

---

### 5️⃣ Compare the implementation complexity of the "down then across" (vertical) printing logic versus the "across" (horizontal) printing logic. Which one requires more pre-calculation and why?
**Answer:**  
The “down then across” logic is more complex because it requires:
- Pre-calculating number of rows and columns.
- Determining index mapping for vertical traversal.
- Computing column width based on filename lengths.

The “across” (horizontal) logic simply iterates linearly through filenames and prints them in sequence, so it needs much less pre-calculation.

---

### 6️⃣ Describe the strategy you used in your code to manage the different display modes (`-l`, `-x`, and default). How did your program decide which function to call for printing?
**Answer:**  
A **mode flag system** was used to select the output format based on user options:
```c
if (flag_long)
    print_long_format(files);
else if (flag_horizontal)
    print_horizontal(files);
else
    print_default(files);
```
This modular design ensures clarity and makes it easy to extend functionality by adding new print functions.

---

### 7️⃣ Why is it necessary to read all directory entries into memory before you can sort them? What are the potential drawbacks of this approach for directories containing millions of files?
**Answer:**  
Sorting requires random access to the entire dataset, so all directory entries must be stored first.  
**Drawbacks:**  
- High memory usage for large directories.  
- Increased I/O time.  
- Potential slowdowns or memory exhaustion if there are millions of files.

---

### 8️⃣ Explain the purpose and signature of the comparison function required by `qsort()`. How does it work, and why must it take `const void *` arguments?
**Answer:**  
`qsort()` is a generic sorting function, so its comparator must accept generic pointers:
```c
int compare(const void *a, const void *b);
```
You cast them to the appropriate type inside:
```c
int compare_names(const void *a, const void *b) {
    const char *sa = *(const char **)a;
    const char *sb = *(const char **)b;
    return strcmp(sa, sb);
}
```
The `const void *` ensures type safety and allows `qsort()` to sort arrays of any data type.

---

### 9️⃣ How do ANSI escape codes work to produce color in a standard Linux terminal? Show the specific code sequence for printing text in green.
**Answer:**  
ANSI escape codes are sequences that modify text appearance.  
They start with `\033[` and end with `m`.  
Example for **green text**:
```c
printf("\033[0;32mThis text is green!\033[0m\n");
```
- `\033[0;32m` → Begin green text  
- `\033[0m` → Reset to default color

---

### 🔟 To color an executable file, you need to check its permission bits. Explain which bits in the `st_mode` field you need to check to determine if a file is executable by the owner, group, or others.
**Answer:**  
You check the **execute permission bits** in `st_mode`:
```c
if (st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))
    printf("Executable file\n");
```
- `S_IXUSR`: Executable by owner  
- `S_IXGRP`: Executable by group  
- `S_IXOTH`: Executable by others  
If any of these bits are set, the file is considered executable.

---

### 1️⃣1️⃣ In a recursive function, what is a "base case"? In the context of your recursive `ls`, what is the base case that stops the recursion from continuing forever?
**Answer:**  
A **base case** is a stopping condition that prevents infinite recursion.  
In `recursive ls`, the base case occurs when:
- A directory has no subdirectories.
- The directory cannot be opened (e.g., permission denied).
Without a base case, the recursion would continue endlessly and crash the program.

---

### 1️⃣2️⃣ Explain why it is essential to construct a full path (e.g., `"parent_dir/subdir"`) before making a recursive call. What would happen if you simply called `do_ls("subdir")` from within `do_ls("parent_dir")`?
**Answer:**  
It’s essential because system calls like `opendir()` and `stat()` require the **correct relative or absolute path** to locate files.  
If you only pass `"subdir"`, the program looks in the **current working directory**, not inside `"parent_dir"`.  
This would cause:
- Wrong file paths.  
- Errors opening subdirectories.  
- Incorrect or incomplete recursion behavior.

---
