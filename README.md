# Minishell
> Minimal UNIX shell built with C

![C](https://img.shields.io/badge/language-C-blue.svg)
![Platform](https://img.shields.io/badge/platform-macOS%20%7C%20Linux-lightgrey.svg)
![Status](https://img.shields.io/badge/status-In%20Progress-orange.svg)

### Key Features

- **Built-in Commands**: 'echo', 'cd', 'pwd', 'export', 'unset', 'env', 'exit'.
- **External Commands**: 'ls', 'cat', 'grep', 'wc' via '$PATH' variable or absolute/relative path.
- **Pipelining**: Supports pipes ('|') through asynchronous child process.
- **Redirection**: Supports input('<'), output('>', '>>'), and Heredoc('<<').
- **Signal Handling**: Supports 'Ctrl-C', 'Ctrl-D' to handle bash signals.
- **Environment Variables**: Supports '$VAR' and handles the '$?' exit status.


### Pipeline Flowchart

```mermaid
graph TD
    Input[Input: readline]
    Lexer[Lexer: tokenization]
    Parser[Parser: AST/command list generation]
    Executor[Executor: command execution]

    Valid{Syntax error?}
    PipeExists{Pipe exists?}
    BuiltIn{Built-in command?}
    Fork{Fork}

    Input --> |char*| Lexer
    Lexer --> |t_token| Parser
    Parser --> Valid
    Valid --> |Yes| SyntaxError[Syntax error message]
    SyntaxError --> Input
    Valid --> |No/AST| Executor
    Executor --> PipeExists

    PipeExists --> |Yes| Fork
    PipeExists --> |No| BuiltIn
    BuiltIn --> |Yes| ParentProcess[Execute in parent process]
    ParentProcess --> Input
    BuiltIn --> |No| Fork
    Fork --> WaitPid[(Waitpid in parent process)]
    WaitPid --> Input
    Fork --> ChildProcess[(Execute in child process)]
    ChildProcess --> Exit[Exit]
```

### Development Roadmap
- **Phase 1**: Environment & Input
    - Parse `envp` and build key-value linked list.
    - Setup input infinite loop.
- **Phase 2**: Lexer & Parser
    - Tokenize `char*` and generate AST.
- **Phase 3**: Executor & Built-ins
- **Phase 4**: Pipes & Redirection
- **Phase 5**: Debugging
