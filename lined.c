/* ------------------------------------------------------
 * LinEd - Full EDLIN-like Line Editor (Patched)
 * Compatible with Turbo C (DOS)
 *
 * Features:
 *  - Original full implementation logic retained
 *  - Zero-padded line numbers: 00000, 00001, …
 *  - Banner with uppercase filename
 *  - Case-insensitive commands
 *  - Status line after every command
 *  - Multi-line insert mode
 *  - Replace/Search with /old/new/[g] syntax
 *  - Range parsing, memory safety, last_a/last_b tracking
 *  - **Each input line is preceded by its current line number**
 * ------------------------------------------------------ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINES 1200
#define LINE_LEN  256
#define INPUT_LEN 512

static char *lines[MAX_LINES];
static int   line_count = 0;
static char  current_file[128] = "";
static int   last_a = 1, last_b = 0; /* last used range; b==0 means unset */

/* -------- utility -------- */
static void chomp(char *s) {
    size_t n = strlen(s);
    if (n && (s[n-1] == '\n' || s[n-1] == '\r')) s[--n] = '\0';
    if (n && s[n-1] == '\r') s[--n] = '\0';
}

/* Lowercase search */
static int strcasestr_pos(const char *hay, const char *needle) {
    size_t i, j, H = strlen(hay), N = strlen(needle);
    if (N == 0) return 0;
    for (i = 0; i + N <= H; ++i) {
        for (j = 0; j < N; ++j) {
            char a = (char)tolower((unsigned char)hay[i+j]);
            char b = (char)tolower((unsigned char)needle[j]);
            if (a != b) break;
        }
        if (j == N) return (int)i;
    }
    return -1;
}

static void to_range_defaults(int *a, int *b) {
    if (*a < 1) *a = 1;
    if (*b < 1 || *b > line_count) *b = line_count;
    if (*a > *b && line_count > 0) { int t=*a; *a=*b; *b=t; }
}

static char *xstrdup(const char *s) {
    size_t n = strlen(s) + 1;
    char *p = (char*)malloc(n);
    if (!p) return NULL;
    memcpy(p, s, n);
    return p;
}

static void free_line(int idx) {
    if (idx >= 0 && idx < line_count && lines[idx]) {
        free(lines[idx]);
        lines[idx] = NULL;
    }
}

/* shift lines up/down to make/remove space */
static int make_room(int pos, int count) {
    int i;
    if (count <= 0) return 1;
    if (line_count + count > MAX_LINES) return 0;
    for (i = line_count - 1; i >= pos; --i) {
        lines[i + count] = lines[i];
    }
    line_count += count;
    return 1;
}

static void close_gap(int start, int count) {
    int i;
    for (i = start; i + count < line_count; ++i) {
        lines[i] = lines[i + count];
    }
    line_count -= count;
}

/* replace old->new in line */
static int replace_in_line(char **s_ptr, const char *oldp, const char *newp, int global) {
    char *s = *s_ptr;
    size_t so = strlen(oldp), sn = strlen(newp);
    int made = 0, limit = 1024;
    if (so == 0) return 0;

    for (;;) {
        char *found = strstr(s, oldp);
        if (!found) break;
        {
            size_t prefix = (size_t)(found - s);
            size_t suffix = strlen(found + so);
            if (prefix + sn + suffix + 1 >= LINE_LEN) break;

            {
                char *buf = (char*)malloc(LINE_LEN);
                if (!buf) break;
                memcpy(buf, s, prefix);
                memcpy(buf + prefix, newp, sn);
                memcpy(buf + prefix + sn, found + so, suffix + 1);
                free(s);
                s = buf;
                *s_ptr = s;
                ++made;
            }
        }
        if (!global) break;
        if (--limit <= 0) break;
    }
    return made;
}

/* parse range like a,b */
static int parse_range(const char *p, int *a, int *b) {
    int x = -1, y = -1;
    const char *c = p;
    while (isspace(*c)) ++c;
    if (*c == '\0') { *a=1; *b=line_count; return 1; }
    if (*c == ',') { ++c; y=atoi(c); *a=1; *b=(y>0?y:line_count); return 1; }
    if (isdigit(*c)) {
        x = atoi(c);
        while (isdigit(*c)) ++c;
        while (isspace(*c)) ++c;
        if (*c == ',') { ++c; while(isspace(*c)) ++c; y=(*c?atoi(c):line_count); }
        else y=x;
        *a = (x>0?x:1); *b = (y>0?y:line_count);
        return 1;
    }
    return 0;
}

static const char* parse_between(const char *p, char delim, char *out, size_t outsz) {
    size_t n=0;
    if (*p != delim) return NULL;
    ++p;
    while (*p && *p != delim) { if (n+1 < outsz) out[n++] = *p; ++p; }
    if (*p != delim) return NULL;
    out[n]='\0';
    return p+1;
}

/* -------- file ops -------- */
static int load_file(const char *name) {
    FILE *f = fopen(name,"rt"); char buf[LINE_LEN]; int i;
    if (!f) return 0;
    for(i=0;i<line_count;++i) free_line(i);
    line_count=0;
    while(fgets(buf,sizeof(buf),f)) {
        chomp(buf);
        if (!(lines[line_count]=xstrdup(buf))) { fclose(f); return 0; }
        if (++line_count>=MAX_LINES) { fclose(f); return 0; }
    }
    fclose(f);
    strncpy(current_file,name,sizeof(current_file)-1); current_file[sizeof(current_file)-1]=0;
    last_a=1; last_b=line_count;
    return 1;
}

static int write_file(const char *name) {
    FILE *f = fopen(name,"wt"); int i;
    if(!f) return 0;
    for(i=0;i<line_count;i++) { fputs(lines[i]?lines[i]:"",f); fputc('\n',f); }
    fclose(f);
    strncpy(current_file,name,sizeof(current_file)-1); current_file[sizeof(current_file)-1]=0;
    return 1;
}

/* -------- commands -------- */
static void cmd_list(int a,int b) {
    int i; to_range_defaults(&a,&b);
    if(line_count==0){puts("(empty)");return;}
    for(i=a;i<=b;i++) { if(i>=1 && i<=line_count) printf("%05d: %s\n",i-1,lines[i-1]?lines[i-1]:""); }
    last_a=a; last_b=b;
}

static void cmd_delete(int a,int b) {
    int count,i; to_range_defaults(&a,&b);
    if(a<1)a=1;if(b>line_count)b=line_count;if(line_count==0||a>b)return;
    for(i=a-1;i<=b-1;i++) free_line(i);
    count=b-a+1; close_gap(a-1,count);
    last_a=a; last_b=(a<=line_count)?a:line_count;
}

static void cmd_insert(int n) {
    char buf[LINE_LEN]; int pos;
    if(n<1||n>line_count+1) n=line_count+1; pos=n-1;
    printf("-- Insert mode at line %05d (end with a single '.') --\n",n-1);
    for(;;){
        printf("%05d: ", pos+1); /* show line number prompt */
        if(!fgets(buf,sizeof(buf),stdin)) break;
        chomp(buf); if(strcmp(buf,".")==0) break;
        if(!make_room(pos,1)) { puts("! out of space"); break; }
        lines[pos]=xstrdup(buf); if(!lines[pos]) { puts("! alloc failed"); break; }
        pos++;
    }
    last_a=n; last_b=pos;
}

static void cmd_edit(int n){
    char buf[LINE_LEN];
    if(n<1||n>line_count){puts("! bad line");return;}
    printf("%05d: %s\n",n-1,lines[n-1]?lines[n-1]:"");
    printf("%05d: ", n);   /* line number before input */
    if(!fgets(buf,sizeof(buf),stdin)) return; chomp(buf);
    free_line(n-1); lines[n-1]=xstrdup(buf); if(!lines[n-1]) puts("! alloc failed");
    last_a=n; last_b=n;
}

static void cmd_replace(int a,int b,const char *spec){
    char oldp[LINE_LEN],newp[LINE_LEN]; int global=0; const char *p=spec; int i,total=0;
    while(isspace(*p))++p;
    p=parse_between(p,'/',oldp,sizeof(oldp)); if(!p){puts("! syntax: R a,b /old/new/[g]");return;}
    while(isspace(*p))++p;
    p=parse_between(p,'/',newp,sizeof(newp)); if(!p){puts("! syntax: R a,b /old/new/[g]");return;}
    while(isspace(*p))++p; if(*p=='g'||*p=='G') global=1;
    to_range_defaults(&a,&b);
    for(i=a;i<=b;i++) if(i>=1&&i<=line_count&&lines[i-1]) total+=replace_in_line(&lines[i-1],oldp,newp,global);
    printf("Replaced %d occurrence(s).\n",total);
    last_a=a; last_b=b;
}

static void cmd_search(int a,int b,const char *spec){
    char pat[LINE_LEN]; const char *p=spec; int i,hits=0;
    while(isspace(*p))++p;
    if(*p=='/') { p=parse_between(p,'/',pat,sizeof(pat)); if(!p){puts("! syntax: S a,b /text/"); return;} }
    else { size_t n=0; while(*p&&isspace(*p))++p; while(*p&&n+1<sizeof(pat)) pat[n++]=*p++; pat[n]=0; }
    to_range_defaults(&a,&b);
    for(i=a;i<=b;i++){ if(i>=1&&i<=line_count&&lines[i-1]){ if(strcasestr_pos(lines[i-1],pat)>=0){printf("%05d: %s\n",i-1,lines[i-1]);hits++;} } }
    printf("-- %d match(es)\n",hits);
    last_a=a; last_b=b;
}

/* -------- REPL & banner -------- */
static void help(void){
    puts("Commands:");
    puts("  L [a][,b]           list lines");
    puts("  I [n]               insert at n (end with a single '.')");
    puts("  D a[,b]             delete lines");
    puts("  E n                 edit (replace) line");
    puts("  R a[,b] /old/new/[g]  replace; 'g' = global per line");
    puts("  S [a][,b] /text/    search (case-insensitive)");
    puts("  O name              open (load) file");
    puts("  W [name]            write (save) file");
    puts("  P                   print status");
    puts("  H or ?              help");
    puts("  Q                   quit");
}

static void status_line(void){
    printf("Lines: %d  File: %s\n", line_count, current_file[0]?current_file:"(none)");
}

static void banner(const char *fname){
    char upper[128]; int i;
    strncpy(upper,fname,sizeof(upper)-1); upper[sizeof(upper)-1]=0;
    for(i=0;upper[i];i++) upper[i]=(char)toupper((unsigned char)upper[i]);
    puts("====================================");
    puts("LinEd - Line Editor Version 1.0a");
    puts("Mickey W. Lawless (C) 2025, 2026");
    printf("Editing: %s\n",upper);
    puts("====================================");
}

static void prompt(void){ printf("* "); fflush(stdout); }

int main(int argc,char **argv){
    char in[INPUT_LEN];

    if(argc>1){ if(!load_file(argv[1])){ printf("! couldn't open '%s' (starting empty)\n",argv[1]); strncpy(current_file,argv[1],sizeof(current_file)-1); current_file[sizeof(current_file)-1]=0; } }

    banner(argc>1?argv[1]:"(none)");
    status_line();

    for(;;){
        char cmd=0; char *p; int a=-1,b=-1,n;
        prompt();
        if(!fgets(in,sizeof(in),stdin)) break; chomp(in); p=in; while(isspace(*p))++p; if(!*p) continue;
        cmd = (char)toupper((unsigned char)*p++); while(isspace(*p))++p;

        switch(cmd){
        case 'L': if(!*p){a=1;b=line_count;} else if(!parse_range(p,&a,&b)){puts("! bad range"); break;} cmd_list(a,b); break;
        case 'I': n=(*p?atoi(p):line_count+1); cmd_insert(n); break;
        case 'D': if(!parse_range(p,&a,&b)){puts("! need D a[,b]"); break;} cmd_delete(a,b); break;
        case 'E': if(!*p){puts("! need E n"); break;} n=atoi(p); cmd_edit(n); break;
        case 'R': { char buf[INPUT_LEN]; const char *spec; int have_range=0; strncpy(buf,p,sizeof(buf)-1); buf[sizeof(buf)-1]=0; spec=strchr(buf,'/'); if(spec){ char tmp[INPUT_LEN]; size_t rlen=(size_t)(spec-buf); if(rlen>=sizeof(tmp))rlen=sizeof(tmp)-1; memcpy(tmp,buf,rlen); tmp[rlen]=0; if(tmp[0]) have_range=parse_range(tmp,&a,&b); else have_range=1; if(!have_range){puts("! bad range"); break;} cmd_replace(have_range?a:1,have_range?b:line_count,spec);} else {puts("! syntax: R a,b /old/new/[g]");} } break;
        case 'O': if(!*p){puts("! need filename"); break;} if(!load_file(p)) puts("! open failed"); else printf("-- loaded %d line(s)\n",line_count); break;
        case 'S': { char buf[INPUT_LEN]; const char *spec; int have_range=0; strncpy(buf,p,sizeof(buf)-1); buf[sizeof(buf)-1]=0; spec=strchr(buf,'/'); if(spec){ char tmp[INPUT_LEN]; size_t rlen=(size_t)(spec-buf); if(rlen>=sizeof(tmp)) rlen=sizeof(tmp)-1; memcpy(tmp,buf,rlen); tmp[rlen]=0; if(tmp[0]) have_range=parse_range(tmp,&a,&b); else have_range=1; if(!have_range){puts("! bad range"); break;} cmd_search(have_range?a:1,have_range?b:line_count,spec);} else cmd_search(1,line_count,p); } break;
        case 'W':
            if(*p){
                if(!write_file(p)) puts("! write failed");
                else printf("-- wrote %d line(s) to %s\n", line_count, p);
            } else if(current_file[0]) {
                if(!write_file(current_file)) puts("! write failed");
                else printf("-- wrote %d line(s) to %s\n", line_count, current_file);
            } else {
                puts("! W needs filename (no current file)");
            }
            break;
        case 'P':
            status_line();
            break;
        case 'H':
        case '?':
            help();
            break;
        case 'Q':
            return 0;
        default:
            puts("?");
            break;
        }
        status_line();
    }

    return 0;
}
