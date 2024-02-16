# cjson

## Lightweight json parser for C projects

```
int cjson_parse(const char *str, const char *fmt, int ops, ...);
```
### Usage
#### `main.c`

```
/*
int cjson_parse(const char *str, const char *fmt, int ops, char *buff, int n);
int cjson_parse(const char *str, const char *fmt, int ops, const char **begin, const char **end);
int cjson_parse(const char *str, const char *fmt, int ops, char **alloc);
*/

int main(int ac, char *av[])
{
 char strbuf[10000];
 int  stat;
 int  fd;
 int  ret;

 (void)ac;
 fd = open(av[1], O_RDONLY);
 ret = read(fd, strbuf, 10000);
 strbuf[ret] = 0;
```

##### For given buffer size

```
 char buff[10000];
 stat = cjson_parse(strbuf, av[2], O_BUFF, buff, 10000);
 printf("%s\n", buff);
```

##### For begin and end pointers

```
 const char *begin;
 const char *end; 
 stat = cjson_parse(strbuf, av[2], O_PTR, &begin, &end);
 while (begin < end)
  printf("%c", *(begin++));
 printf("\n");
```

##### For a new string from the heap

```
 char *str;
 stat = cjson_parse(strbuf, av[2], O_ALLOC, &str);
 if (0 <= stat)
 {
  printf("%s\n", str);
  free(str);
 }
```

```
 return (stat);
}
```

#### `tmp.json`

```
{
 "id": 14433,
 "name": "api-red",
 "description": "",
 "image": null,
 "website": "",
 "public": true,
 "scopes": [
  "public",
  "projects",
  "profile",
  "elearning",
  "tig",
  "forum"
 ],
}
```

##### Run

```
./a.out tmp.json .scopes[0]
```

##### Output

`"public"`

#### `tmp2.json`

```
[
 {
  "id": 1,
  "api-name": "t1",
 },
 {
  "id": 2,
  "api-name": "t1",
 },
 {
  "id": 2,
  "api-name": "t1",
 }
]
```

##### Run

```
./a.out tmp2.json [1].id
```

##### Output

`2`
