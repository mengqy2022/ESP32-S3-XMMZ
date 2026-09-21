import os
import re
import sys

def strip(src):
    src = re.sub(r'/\*.*?\*/', '', src, flags=re.S)
    src = re.sub(r'//[^\n]*', '', src)
    out = []
    i = 0
    n = len(src)
    while i < n:
        c = src[i]
        if c == '"':
            j = i + 1
            while j < n:
                if src[j] == '\\':
                    j += 2
                    continue
                if src[j] == '"':
                    break
                j += 1
            i = j + 1
        elif c == "'":
            j = i + 1
            while j < n:
                if src[j] == '\\':
                    j += 2
                    continue
                if src[j] == "'":
                    break
                j += 1
            i = j + 1
        else:
            out.append(c)
            i += 1
    return ''.join(out)

pairs = {')': '(', '}': '{', ']': '['}


def check(f):
    try:
        s = strip(open(f, encoding='utf-8').read())
    except UnicodeDecodeError:
        # 生成的字体源是二进制/超大 gb18030 无关内容, 跳过解析失败的文件
        print("  SKIP (binary?) %s" % f)
        return True
    stack = []
    for ch in s:
        if ch in '({[':
            stack.append(ch)
        elif ch in ')}]':
            if not stack or stack[-1] != pairs[ch]:
                print("  MISMATCH at %r in %s" % (ch, f))
                return False
            stack.pop()
    if stack:
        print("  UNCLOSED %s in %s" % (''.join(stack), f))
        return False
    print("  OK %s" % f)
    return True


def main():
    files = sys.argv[1:]
    if not files:
        roots = ['main']
        exts = ('.c', '.h')
        files = []
        for root in roots:
            for dp, _, fns in os.walk(root):
                for fn in fns:
                    if fn.endswith(exts):
                        files.append(os.path.join(dp, fn))
    allok = True
    for f in sorted(files):
        if not check(f):
            allok = False
    print("ALL OK" if allok else "FAIL")


if __name__ == '__main__':
    main()