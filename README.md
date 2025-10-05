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
