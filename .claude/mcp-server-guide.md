# Building a Libdragon MCP Server - Complete Guide

This document explains how to build a Model Context Protocol (MCP) server for Libdragon development. An MCP server provides Claude Code with structured access to documentation, code examples, and community projects.

## Table of Contents
1. [What is MCP?](#what-is-mcp)
2. [Why Build an MCP Server for Libdragon?](#why-build-an-mcp-server-for-libdragon)
3. [Architecture Overview](#architecture-overview)
4. [Prerequisites](#prerequisites)
5. [Implementation Guide](#implementation-guide)
6. [Integration with Claude Code](#integration-with-claude-code)
7. [Advanced Features](#advanced-features)
8. [Maintenance and Updates](#maintenance-and-updates)

---

## What is MCP?

**Model Context Protocol (MCP)** is an open standard that allows AI assistants like Claude Code to access external data sources and tools. Think of it as an API that Claude Code can call to get information.

### Key Concepts

- **Server**: A program that provides data/tools to Claude Code
- **Resources**: Static or dynamic data (files, documentation, code examples)
- **Tools**: Functions Claude Code can call (search, query, analyze)
- **Prompts**: Reusable prompt templates

### MCP vs Other Approaches

| Approach | Best For | Limitations |
|----------|----------|-------------|
| **Static files** (.claude/context/) | Small, stable datasets | Manual updates, no search |
| **WebFetch** | Live documentation | Slow, rate-limited |
| **MCP Server** | Large code repos, search | Setup complexity |
| **MCP + Vector DB** | Semantic search | High complexity, overkill for most |

---

## Why Build an MCP Server for Libdragon?

### Problems It Solves

1. **Niche SDK**: Libdragon isn't well-represented in Claude's training data
2. **Code Examples**: Need to search across multiple community projects
3. **Fast Access**: Avoid WebFetch delays when querying docs/examples
4. **Pattern Discovery**: Find similar implementations across projects

### When to Build It

Build an MCP server if you:
- Ask "show me examples of X" frequently
- Need to search across multiple repos simultaneously
- Want instant access to community code patterns
- Plan multiple N64 projects using these resources

Don't build it if:
- You're happy with WebFetch speed
- This is a one-off project
- You don't need to search code examples often

---

## Architecture Overview

### High-Level Design

```
┌─────────────────────────────────────────────────────────────┐
│                         Claude Code                         │
│  (Asks: "Show me sprite rendering examples")                │
└────────────────────────┬────────────────────────────────────┘
                         │ MCP Protocol
                         ▼
┌─────────────────────────────────────────────────────────────┐
│                   Libdragon MCP Server                      │
│  ┌───────────────┐  ┌────────────────┐  ┌────────────────┐ │
│  │   Resources   │  │     Tools      │  │    Prompts     │ │
│  │ - API Docs    │  │ - Search       │  │ - Find pattern │ │
│  │ - Examples    │  │ - List files   │  │ - Debug help   │ │
│  │ - Wiki        │  │ - Get content  │  │ - API lookup   │ │
│  └───────────────┘  └────────────────┘  └────────────────┘ │
└────────────────────────┬────────────────────────────────────┘
                         │ File System
                         ▼
┌─────────────────────────────────────────────────────────────┐
│                    Indexed Repositories                     │
│  ~/libdragon-mcp-data/                                      │
│  ├── libdragon/          (Official SDK)                     │
│  ├── tiny3d/             (3D library)                       │
│  ├── projects/                                              │
│  │   ├── BrewChristmas/ (Tech demo)                        │
│  │   ├── DrivingStrikers64/ (Full game)                    │
│  │   └── ... (other projects)                              │
│  └── n64brew-wiki/       (Documentation)                   │
└─────────────────────────────────────────────────────────────┘
```

### What We'll Build

**Phase 1: Basic MCP Server** (Recommended Start)
- Resources exposing code repositories
- Simple search tool across indexed projects
- File listing and content retrieval

**Phase 2: Enhanced Search** (Optional)
- Pattern matching for specific APIs
- Category-based search (graphics, audio, input)
- Example extraction with context

**Phase 3: Semantic Search** (Advanced, Optional)
- Vector embeddings for code
- "Find similar implementations" capability
- Requires vector database (ChromaDB, Pinecone, etc.)

---

## Prerequisites

### System Requirements
- **Node.js 18+** or **Python 3.10+** (choose one)
- **Git** (for cloning repos)
- **Disk space**: ~500MB for all indexed repos

### Knowledge Requirements
- Basic programming (JavaScript/TypeScript or Python)
- Understanding of REST APIs or RPC concepts
- Familiarity with file system operations

### Choose Your Stack

**Option A: TypeScript (Recommended)**
- Official MCP SDK with TypeScript support
- Better type safety
- Easier debugging with VS Code

**Option B: Python**
- Simpler for quick prototypes
- Good libraries for text processing
- Easier to add vector search later (with langchain)

This guide uses **TypeScript** for examples.

---

## Implementation Guide

### Step 1: Set Up Project Structure

```bash
# Create project directory
mkdir ~/libdragon-mcp-server
cd ~/libdragon-mcp-server

# Initialize Node.js project
npm init -y

# Install MCP SDK
npm install @modelcontextprotocol/sdk

# Install additional dependencies
npm install zod        # Schema validation
npm install glob       # File pattern matching
npm install ripgrep    # Fast code search (optional)

# Create TypeScript config
npx tsc --init
```

**Project structure:**
```
libdragon-mcp-server/
├── package.json
├── tsconfig.json
├── src/
│   ├── index.ts           # Main server entry point
│   ├── resources.ts       # Resource definitions
│   ├── tools.ts           # Tool implementations
│   └── indexer.ts         # Repository indexer
├── data/
│   ├── libdragon/         # Cloned repos (gitignored)
│   ├── tiny3d/
│   └── projects/
└── index/
    └── file-index.json    # Pre-built index for fast search
```

### Step 2: Clone and Index Repositories

Create a setup script to clone all relevant repositories:

```typescript
// src/indexer.ts
import { execSync } from 'child_process';
import * as fs from 'fs';
import * as path from 'path';
import { glob } from 'glob';

interface IndexedFile {
  path: string;
  repo: string;
  type: 'source' | 'example' | 'docs';
  language?: string;
}

const REPOS = [
  // Official SDK
  {
    url: 'https://github.com/DragonMinded/libdragon',
    path: 'libdragon',
    branch: 'preview',
    type: 'sdk'
  },
  // Libraries
  {
    url: 'https://github.com/HailToDodongo/tiny3d',
    path: 'tiny3d',
    type: 'library'
  },
  // Complete games (Tiny3D)
  {
    url: 'https://github.com/SpookyIluha/Driving-Strikers-64',
    path: 'projects/driving-strikers-64',
    type: 'game-3d'
  },
  {
    url: 'https://github.com/SpookyIluha/CounterEmotion-Bar',
    path: 'projects/counteremotion-bar',
    type: 'game-3d'
  },
  {
    url: 'https://github.com/SpookyIluha/SpaceWavesN64',
    path: 'projects/spacewaves-n64',
    type: 'game-3d'
  },
  {
    url: 'https://github.com/RosieSapphire/Hungover',
    path: 'projects/hungover',
    type: 'game-3d'
  },
  // 2D/Other games
  {
    url: 'https://github.com/SpookyIluha/DDLC64-LibdragonVNE',
    path: 'projects/ddlc64',
    type: 'game-2d'
  },
  {
    url: 'https://github.com/SpookyIluha/Super-Haxagon64',
    path: 'projects/super-haxagon64',
    type: 'game-2d'
  },
  {
    url: 'https://github.com/SpookyIluha/StarshipMadness64',
    path: 'projects/starship-madness64',
    type: 'game-2d'
  },
  {
    url: 'https://github.com/RosieSapphire/FNaF64',
    path: 'projects/fnaf64',
    type: 'game-2d'
  },
  // Tech demos
  {
    url: 'https://github.com/SpookyIluha/BrewChristmas',
    path: 'projects/brew-christmas',
    type: 'demo'
  },
  {
    url: 'https://github.com/SpookyIluha/Brew-SkydomeN64',
    path: 'projects/brew-skydome',
    type: 'demo'
  },
  {
    url: 'https://github.com/SpookyIluha/BrewReality',
    path: 'projects/brew-reality',
    type: 'demo'
  },
  // Community
  {
    url: 'https://github.com/n64brew/N64brew-GameJam2024',
    path: 'projects/game-jam-2024',
    type: 'collection'
  }
];

export class RepositoryIndexer {
  private dataDir: string;
  private indexFile: string;

  constructor(dataDir: string = './data', indexFile: string = './index/file-index.json') {
    this.dataDir = dataDir;
    this.indexFile = indexFile;
  }

  /**
   * Clone or update all repositories
   */
  async cloneRepositories(): Promise<void> {
    console.log('Cloning/updating repositories...');

    if (!fs.existsSync(this.dataDir)) {
      fs.mkdirSync(this.dataDir, { recursive: true });
    }

    for (const repo of REPOS) {
      const repoPath = path.join(this.dataDir, repo.path);

      if (fs.existsSync(repoPath)) {
        console.log(`Updating ${repo.path}...`);
        execSync('git pull', { cwd: repoPath, stdio: 'inherit' });
      } else {
        console.log(`Cloning ${repo.url} to ${repo.path}...`);
        const parentDir = path.dirname(repoPath);
        if (!fs.existsSync(parentDir)) {
          fs.mkdirSync(parentDir, { recursive: true });
        }

        const branch = repo.branch ? `-b ${repo.branch}` : '';
        execSync(
          `git clone ${branch} --depth 1 ${repo.url} ${repoPath}`,
          { stdio: 'inherit' }
        );
      }
    }
  }

  /**
   * Build searchable index of all files
   */
  async buildIndex(): Promise<void> {
    console.log('Building file index...');

    const index: IndexedFile[] = [];

    for (const repo of REPOS) {
      const repoPath = path.join(this.dataDir, repo.path);

      // Index C/C++ source files
      const sourceFiles = await glob('**/*.{c,h,cpp,hpp}', {
        cwd: repoPath,
        ignore: ['**/build/**', '**/node_modules/**', '**/.git/**']
      });

      for (const file of sourceFiles) {
        index.push({
          path: path.join(repoPath, file),
          repo: repo.path,
          type: this.categorizeFile(file, repo.type),
          language: this.getLanguage(file)
        });
      }

      // Index documentation
      const docFiles = await glob('**/*.{md,txt,rst}', {
        cwd: repoPath,
        ignore: ['**/node_modules/**', '**/.git/**']
      });

      for (const file of docFiles) {
        index.push({
          path: path.join(repoPath, file),
          repo: repo.path,
          type: 'docs',
          language: 'markdown'
        });
      }
    }

    // Write index to file
    const indexDir = path.dirname(this.indexFile);
    if (!fs.existsSync(indexDir)) {
      fs.mkdirSync(indexDir, { recursive: true });
    }

    fs.writeFileSync(this.indexFile, JSON.stringify(index, null, 2));
    console.log(`Indexed ${index.length} files`);
  }

  private categorizeFile(filePath: string, repoType: string): 'source' | 'example' | 'docs' {
    if (filePath.includes('/examples/') || filePath.includes('/example/')) {
      return 'example';
    }
    if (filePath.includes('/docs/') || filePath.includes('/doc/')) {
      return 'docs';
    }
    return 'source';
  }

  private getLanguage(filePath: string): string {
    const ext = path.extname(filePath);
    if (ext === '.c' || ext === '.h') return 'c';
    if (ext === '.cpp' || ext === '.hpp') return 'cpp';
    if (ext === '.md') return 'markdown';
    return 'text';
  }
}

// CLI for running indexer
if (require.main === module) {
  const indexer = new RepositoryIndexer();

  (async () => {
    await indexer.cloneRepositories();
    await indexer.buildIndex();
    console.log('Done!');
  })();
}
```

**Run the indexer:**
```bash
npx ts-node src/indexer.ts
```

This will:
1. Clone all repositories to `data/` directory
2. Build a searchable index in `index/file-index.json`
3. Take ~5-10 minutes on first run (downloading repos)

### Step 3: Define Resources

Resources are static data that Claude Code can access:

```typescript
// src/resources.ts
import { Resource } from '@modelcontextprotocol/sdk/types.js';
import * as fs from 'fs';
import * as path from 'path';

interface FileIndex {
  path: string;
  repo: string;
  type: string;
  language?: string;
}

export class LibdragonResources {
  private index: FileIndex[];

  constructor(indexPath: string = './index/file-index.json') {
    this.index = JSON.parse(fs.readFileSync(indexPath, 'utf-8'));
  }

  /**
   * List all available resources
   */
  listResources(): Resource[] {
    return [
      {
        uri: 'libdragon://index',
        name: 'File Index',
        description: 'Complete index of all files in Libdragon ecosystem',
        mimeType: 'application/json'
      },
      {
        uri: 'libdragon://examples/all',
        name: 'All Examples',
        description: 'All example code files from official SDK and community projects',
        mimeType: 'text/plain'
      },
      {
        uri: 'libdragon://api/reference',
        name: 'API Reference',
        description: 'Libdragon API documentation',
        mimeType: 'text/markdown'
      },
      {
        uri: 'libdragon://projects/3d',
        name: '3D Game Projects',
        description: 'Complete games using Tiny3D',
        mimeType: 'application/json'
      },
      {
        uri: 'libdragon://projects/2d',
        name: '2D Game Projects',
        description: 'Complete 2D games',
        mimeType: 'application/json'
      }
    ];
  }

  /**
   * Get resource content by URI
   */
  async getResource(uri: string): Promise<string> {
    if (uri === 'libdragon://index') {
      return JSON.stringify(this.index, null, 2);
    }

    if (uri === 'libdragon://examples/all') {
      const examples = this.index.filter(f => f.type === 'example');
      return JSON.stringify(examples, null, 2);
    }

    if (uri === 'libdragon://projects/3d') {
      const games = this.index.filter(f =>
        f.repo.includes('driving-strikers') ||
        f.repo.includes('spacewaves') ||
        f.repo.includes('hungover') ||
        f.repo.includes('counteremotion')
      );
      return JSON.stringify(this.groupByRepo(games), null, 2);
    }

    if (uri === 'libdragon://projects/2d') {
      const games = this.index.filter(f =>
        f.repo.includes('ddlc64') ||
        f.repo.includes('haxagon') ||
        f.repo.includes('starship') ||
        f.repo.includes('fnaf64')
      );
      return JSON.stringify(this.groupByRepo(games), null, 2);
    }

    throw new Error(`Unknown resource URI: ${uri}`);
  }

  private groupByRepo(files: FileIndex[]): Record<string, FileIndex[]> {
    return files.reduce((acc, file) => {
      if (!acc[file.repo]) acc[file.repo] = [];
      acc[file.repo].push(file);
      return acc;
    }, {} as Record<string, FileIndex[]>);
  }
}
```

### Step 4: Implement Search Tools

Tools are functions Claude Code can call:

```typescript
// src/tools.ts
import { Tool } from '@modelcontextprotocol/sdk/types.js';
import * as fs from 'fs';
import * as path from 'path';
import { execSync } from 'child_process';

interface FileIndex {
  path: string;
  repo: string;
  type: string;
  language?: string;
}

export class LibdragonTools {
  private index: FileIndex[];
  private dataDir: string;

  constructor(indexPath: string = './index/file-index.json', dataDir: string = './data') {
    this.index = JSON.parse(fs.readFileSync(indexPath, 'utf-8'));
    this.dataDir = dataDir;
  }

  /**
   * Define available tools
   */
  listTools(): Tool[] {
    return [
      {
        name: 'search_code',
        description: 'Search for code patterns across all Libdragon projects',
        inputSchema: {
          type: 'object',
          properties: {
            query: {
              type: 'string',
              description: 'Search query (regex supported)'
            },
            fileType: {
              type: 'string',
              enum: ['c', 'h', 'cpp', 'all'],
              description: 'File type to search'
            },
            category: {
              type: 'string',
              enum: ['sdk', 'examples', 'games-3d', 'games-2d', 'demos', 'all'],
              description: 'Category to search within'
            }
          },
          required: ['query']
        }
      },
      {
        name: 'get_file_content',
        description: 'Get the content of a specific file',
        inputSchema: {
          type: 'object',
          properties: {
            path: {
              type: 'string',
              description: 'File path from index'
            }
          },
          required: ['path']
        }
      },
      {
        name: 'find_api_usage',
        description: 'Find examples of specific Libdragon API usage',
        inputSchema: {
          type: 'object',
          properties: {
            apiFunction: {
              type: 'string',
              description: 'API function name (e.g., rdp_attach_display, sprite_load)'
            },
            includeContext: {
              type: 'boolean',
              description: 'Include surrounding code context'
            }
          },
          required: ['apiFunction']
        }
      },
      {
        name: 'list_files_by_pattern',
        description: 'List files matching a specific pattern or in a category',
        inputSchema: {
          type: 'object',
          properties: {
            pattern: {
              type: 'string',
              description: 'File name pattern (e.g., *sprite*, *audio*)'
            },
            category: {
              type: 'string',
              enum: ['sdk', 'examples', 'games', 'demos', 'all']
            }
          }
        }
      }
    ];
  }

  /**
   * Search code across all indexed files
   */
  async searchCode(query: string, fileType: string = 'all', category: string = 'all'): Promise<any> {
    // Filter files by category and type
    let filteredFiles = this.index;

    if (fileType !== 'all') {
      filteredFiles = filteredFiles.filter(f => f.language === fileType);
    }

    if (category !== 'all') {
      filteredFiles = this.filterByCategory(filteredFiles, category);
    }

    // Use ripgrep for fast searching
    const results: Array<{file: string, line: number, content: string, repo: string}> = [];

    for (const file of filteredFiles) {
      try {
        // Use ripgrep (fast) or fallback to grep
        const output = execSync(
          `rg -n "${query}" "${file.path}" || grep -n "${query}" "${file.path}" || true`,
          { encoding: 'utf-8', maxBuffer: 1024 * 1024 * 10 }
        );

        if (output.trim()) {
          const lines = output.trim().split('\n');
          for (const line of lines) {
            const match = line.match(/^(\d+):(.*)$/);
            if (match) {
              results.push({
                file: file.path,
                line: parseInt(match[1]),
                content: match[2].trim(),
                repo: file.repo
              });
            }
          }
        }
      } catch (err) {
        // File doesn't contain pattern, skip
      }
    }

    return {
      query,
      matchCount: results.length,
      matches: results.slice(0, 50) // Limit to 50 results
    };
  }

  /**
   * Get file content
   */
  async getFileContent(filePath: string): Promise<string> {
    if (!fs.existsSync(filePath)) {
      throw new Error(`File not found: ${filePath}`);
    }
    return fs.readFileSync(filePath, 'utf-8');
  }

  /**
   * Find API usage examples
   */
  async findApiUsage(apiFunction: string, includeContext: boolean = false): Promise<any> {
    const pattern = `\\b${apiFunction}\\b`;
    const results = await this.searchCode(pattern, 'c', 'all');

    if (includeContext && results.matches.length > 0) {
      // Add surrounding lines for context
      for (const match of results.matches) {
        const content = await this.getFileContent(match.file);
        const lines = content.split('\n');
        const startLine = Math.max(0, match.line - 5);
        const endLine = Math.min(lines.length, match.line + 5);

        match['context'] = lines.slice(startLine, endLine).join('\n');
      }
    }

    return results;
  }

  /**
   * List files by pattern
   */
  async listFilesByPattern(pattern?: string, category: string = 'all'): Promise<any> {
    let filtered = this.index;

    if (category !== 'all') {
      filtered = this.filterByCategory(filtered, category);
    }

    if (pattern) {
      const regex = new RegExp(pattern.replace(/\*/g, '.*'), 'i');
      filtered = filtered.filter(f => regex.test(path.basename(f.path)));
    }

    return {
      count: filtered.length,
      files: filtered.slice(0, 100) // Limit to 100
    };
  }

  private filterByCategory(files: FileIndex[], category: string): FileIndex[] {
    switch (category) {
      case 'sdk':
        return files.filter(f => f.repo === 'libdragon' || f.repo === 'tiny3d');
      case 'examples':
        return files.filter(f => f.type === 'example');
      case 'games-3d':
        return files.filter(f =>
          f.repo.includes('driving-strikers') ||
          f.repo.includes('spacewaves') ||
          f.repo.includes('hungover') ||
          f.repo.includes('counteremotion')
        );
      case 'games-2d':
        return files.filter(f =>
          f.repo.includes('ddlc64') ||
          f.repo.includes('haxagon') ||
          f.repo.includes('starship') ||
          f.repo.includes('fnaf64')
        );
      case 'demos':
        return files.filter(f =>
          f.repo.includes('brew-christmas') ||
          f.repo.includes('brew-skydome') ||
          f.repo.includes('brew-reality')
        );
      default:
        return files;
    }
  }
}
```

### Step 5: Create MCP Server

Tie everything together in the main server:

```typescript
// src/index.ts
import { Server } from '@modelcontextprotocol/sdk/server/index.js';
import { StdioServerTransport } from '@modelcontextprotocol/sdk/server/stdio.js';
import {
  ListResourcesRequestSchema,
  ReadResourceRequestSchema,
  ListToolsRequestSchema,
  CallToolRequestSchema,
} from '@modelcontextprotocol/sdk/types.js';

import { LibdragonResources } from './resources.js';
import { LibdragonTools } from './tools.js';

class LibdragonMCPServer {
  private server: Server;
  private resources: LibdragonResources;
  private tools: LibdragonTools;

  constructor() {
    this.server = new Server(
      {
        name: 'libdragon-mcp-server',
        version: '1.0.0',
      },
      {
        capabilities: {
          resources: {},
          tools: {},
        },
      }
    );

    this.resources = new LibdragonResources();
    this.tools = new LibdragonTools();

    this.setupHandlers();
  }

  private setupHandlers() {
    // List available resources
    this.server.setRequestHandler(ListResourcesRequestSchema, async () => {
      return {
        resources: this.resources.listResources(),
      };
    });

    // Read resource content
    this.server.setRequestHandler(ReadResourceRequestSchema, async (request) => {
      const content = await this.resources.getResource(request.params.uri);
      return {
        contents: [
          {
            uri: request.params.uri,
            mimeType: 'text/plain',
            text: content,
          },
        ],
      };
    });

    // List available tools
    this.server.setRequestHandler(ListToolsRequestSchema, async () => {
      return {
        tools: this.tools.listTools(),
      };
    });

    // Call tool
    this.server.setRequestHandler(CallToolRequestSchema, async (request) => {
      const { name, arguments: args } = request.params;

      switch (name) {
        case 'search_code':
          const searchResults = await this.tools.searchCode(
            args.query as string,
            args.fileType as string,
            args.category as string
          );
          return {
            content: [
              {
                type: 'text',
                text: JSON.stringify(searchResults, null, 2),
              },
            ],
          };

        case 'get_file_content':
          const content = await this.tools.getFileContent(args.path as string);
          return {
            content: [
              {
                type: 'text',
                text: content,
              },
            ],
          };

        case 'find_api_usage':
          const apiResults = await this.tools.findApiUsage(
            args.apiFunction as string,
            args.includeContext as boolean
          );
          return {
            content: [
              {
                type: 'text',
                text: JSON.stringify(apiResults, null, 2),
              },
            ],
          };

        case 'list_files_by_pattern':
          const fileList = await this.tools.listFilesByPattern(
            args.pattern as string,
            args.category as string
          );
          return {
            content: [
              {
                type: 'text',
                text: JSON.stringify(fileList, null, 2),
              },
            ],
          };

        default:
          throw new Error(`Unknown tool: ${name}`);
      }
    });
  }

  async run() {
    const transport = new StdioServerTransport();
    await this.server.connect(transport);
    console.error('Libdragon MCP Server running on stdio');
  }
}

// Start server
const server = new LibdragonMCPServer();
server.run().catch(console.error);
```

### Step 6: Add Build Scripts

Update `package.json`:

```json
{
  "name": "libdragon-mcp-server",
  "version": "1.0.0",
  "type": "module",
  "scripts": {
    "build": "tsc",
    "start": "node dist/index.js",
    "index": "ts-node src/indexer.ts",
    "dev": "ts-node src/index.ts"
  },
  "dependencies": {
    "@modelcontextprotocol/sdk": "^0.5.0",
    "glob": "^10.3.10",
    "zod": "^3.22.4"
  },
  "devDependencies": {
    "@types/node": "^20.11.0",
    "ts-node": "^10.9.2",
    "typescript": "^5.3.3"
  }
}
```

**Build and run:**
```bash
# Build TypeScript
npm run build

# Run indexer (first time only)
npm run index

# Start server
npm start
```

---

## Integration with Claude Code

### Configure Claude Code to Use Your MCP Server

Create or edit `~/Library/Application Support/Claude/claude_desktop_config.json` (macOS):

```json
{
  "mcpServers": {
    "libdragon": {
      "command": "node",
      "args": [
        "/Users/josh/libdragon-mcp-server/dist/index.js"
      ],
      "env": {}
    }
  }
}
```

**For Windows:** `%APPDATA%\Claude\claude_desktop_config.json`
**For Linux:** `~/.config/Claude/claude_desktop_config.json`

### Restart Claude Code

After updating the config:
1. Quit Claude Code completely
2. Restart Claude Code
3. The MCP server should auto-connect

### Verify Connection

In Claude Code, you should see:
- New tools available: `search_code`, `find_api_usage`, etc.
- New resources: `libdragon://` URIs

### Example Usage

Now you can ask Claude Code:

**"Find examples of sprite rendering"**
```
Claude will use: search_code(query: "sprite_load|rdp_draw_sprite", category: "games")
```

**"Show me how Driving Strikers handles audio"**
```
Claude will use: search_code(query: "audio_init|wav64", category: "games-3d")
Then: get_file_content(path: "...")
```

**"Find all uses of rdp_attach_display"**
```
Claude will use: find_api_usage(apiFunction: "rdp_attach_display", includeContext: true)
```

---

## Advanced Features

### Feature 1: Pattern Templates

Add pre-defined search patterns:

```typescript
// src/patterns.ts
export const COMMON_PATTERNS = {
  'sprite-rendering': 'rdp_draw_sprite|sprite_load|rdp_enable_texture_copy',
  'audio-setup': 'audio_init|wav64_open|xm64player',
  'input-handling': 'controller_scan|get_keys_down|get_keys_held',
  '3d-rendering': 't3d_init|t3d_model_load|t3d_viewport',
  'display-setup': 'display_init|display_lock|display_show',
};

// Add as a tool
{
  name: 'search_pattern',
  description: 'Search for common Libdragon patterns',
  inputSchema: {
    type: 'object',
    properties: {
      pattern: {
        type: 'string',
        enum: Object.keys(COMMON_PATTERNS),
        description: 'Pre-defined pattern to search for'
      }
    }
  }
}
```

### Feature 2: Code Context Extraction

Extract complete function definitions:

```typescript
async extractFunction(filePath: string, functionName: string): Promise<string> {
  const content = fs.readFileSync(filePath, 'utf-8');
  const lines = content.split('\n');

  // Find function definition
  const startRegex = new RegExp(`^\\w+\\s+${functionName}\\s*\\(`);
  let startLine = -1;

  for (let i = 0; i < lines.length; i++) {
    if (startRegex.test(lines[i])) {
      startLine = i;
      break;
    }
  }

  if (startLine === -1) return '';

  // Find matching closing brace
  let braceCount = 0;
  let endLine = startLine;

  for (let i = startLine; i < lines.length; i++) {
    braceCount += (lines[i].match(/{/g) || []).length;
    braceCount -= (lines[i].match(/}/g) || []).length;

    if (braceCount === 0 && i > startLine) {
      endLine = i;
      break;
    }
  }

  return lines.slice(startLine, endLine + 1).join('\n');
}
```

### Feature 3: Semantic Search (Advanced)

For "find similar code" queries, add vector search:

```bash
npm install @pinecone-database/pinecone openai
```

```typescript
import { Pinecone } from '@pinecone-database/pinecone';
import OpenAI from 'openai';

class SemanticCodeSearch {
  private pinecone: Pinecone;
  private openai: OpenAI;
  private indexName = 'libdragon-code';

  constructor() {
    this.pinecone = new Pinecone({ apiKey: process.env.PINECONE_API_KEY });
    this.openai = new OpenAI({ apiKey: process.env.OPENAI_API_KEY });
  }

  async indexCode(files: FileIndex[]): Promise<void> {
    const index = this.pinecone.index(this.indexName);

    for (const file of files) {
      const content = fs.readFileSync(file.path, 'utf-8');

      // Split into functions/chunks
      const chunks = this.splitIntoFunctions(content);

      for (const chunk of chunks) {
        // Generate embedding
        const embedding = await this.openai.embeddings.create({
          model: 'text-embedding-3-small',
          input: chunk.code,
        });

        // Store in Pinecone
        await index.upsert([{
          id: `${file.path}:${chunk.name}`,
          values: embedding.data[0].embedding,
          metadata: {
            file: file.path,
            repo: file.repo,
            function: chunk.name,
            code: chunk.code
          }
        }]);
      }
    }
  }

  async findSimilar(query: string, topK: number = 5): Promise<any[]> {
    // Generate query embedding
    const queryEmbedding = await this.openai.embeddings.create({
      model: 'text-embedding-3-small',
      input: query,
    });

    // Search Pinecone
    const index = this.pinecone.index(this.indexName);
    const results = await index.query({
      vector: queryEmbedding.data[0].embedding,
      topK,
      includeMetadata: true
    });

    return results.matches;
  }
}
```

**Note:** Semantic search adds complexity and costs (OpenAI API, Pinecone). Only add if you need "find similar implementations" capability.

---

## Maintenance and Updates

### Updating Repositories

Run the indexer periodically to update:

```bash
cd ~/libdragon-mcp-server
npm run index
```

This will:
1. Pull latest changes from all repos
2. Rebuild the file index

**Automate with cron (optional):**
```bash
# Update weekly
0 2 * * 0 cd ~/libdragon-mcp-server && npm run index
```

### Adding New Repositories

Edit `src/indexer.ts` and add to `REPOS` array:

```typescript
{
  url: 'https://github.com/username/new-n64-project',
  path: 'projects/new-project',
  type: 'game-3d'
}
```

Then re-run indexer:
```bash
npm run index
```

### Monitoring Performance

Log search performance:

```typescript
async searchCode(query: string, ...args): Promise<any> {
  const startTime = Date.now();
  const results = await this.searchCodeImpl(query, ...args);
  const duration = Date.now() - startTime;

  console.error(`Search took ${duration}ms, found ${results.matchCount} results`);
  return results;
}
```

### Troubleshooting

**MCP server not connecting:**
1. Check config file path is correct
2. Verify `command` points to built JS file (not TS)
3. Look at Claude Code logs (Help → Show Logs)

**Slow searches:**
1. Reduce index size (exclude non-essential files)
2. Use ripgrep instead of grep
3. Limit results returned (already implemented)

**Out of memory:**
1. Increase Node.js heap: `NODE_OPTIONS=--max-old-space-size=4096 npm start`
2. Index fewer repositories
3. Stream large results instead of loading all at once

---

## Comparison: With vs Without MCP Server

### Without MCP (Current Setup)

**User asks:** "Show me examples of sprite rendering in Libdragon games"

**Claude Code:**
1. WebFetch libdragon docs (10-20 seconds)
2. Asks you which projects to check
3. You provide GitHub URLs
4. WebFetch each project (10-20 seconds each)
5. Manually searches through code
6. Shows limited examples

**Total time:** 2-5 minutes, manual input required

### With MCP Server

**User asks:** "Show me examples of sprite rendering in Libdragon games"

**Claude Code:**
1. Calls `search_code(query: "sprite_load|rdp_draw_sprite", category: "games")`
2. Gets instant results from indexed repos
3. Calls `get_file_content()` for best matches
4. Shows examples with context

**Total time:** 5-10 seconds, fully automatic

---

## Next Steps

1. **Start Simple**: Build basic MCP server with resources and search
2. **Test with Real Questions**: Use it for actual development, see what's missing
3. **Add Features Incrementally**: Don't build everything at once
4. **Consider Semantic Search**: Only if you frequently ask "find similar" questions

## Resources

- **MCP Documentation**: https://modelcontextprotocol.io/
- **MCP SDK**: https://github.com/modelcontextprotocol/typescript-sdk
- **Example Servers**: https://github.com/modelcontextprotocol/servers
- **Claude Code MCP Guide**: https://docs.claude.com/claude-code/mcp

---

This guide gives you everything needed to build a production-ready Libdragon MCP server. Start with Phase 1 (basic server), test it thoroughly, then add advanced features only if you need them.
