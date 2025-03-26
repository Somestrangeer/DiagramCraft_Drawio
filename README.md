# DiagramCraft: DevOps Diagram Generator 🚀

**A graduation project for the University**  
*Streamlining DevOps diagram creation with AI-powered automation*

## 🌟 Overview
DiagramCraft is a web-based tool designed to **save DevOps teams countless hours** by automating the creation of complex diagrams. Instead of manually designing diagrams, users can:

1. **Write diagrams using Mermaid syntax** (text-to-diagram)  
2. **Generate diagrams using AI** (DeepSeek) by describing what they need in natural language

## ✨ Key Features
- **Mermaid.js Integration**: Create diagrams using simple text syntax
- **AI-Powered Generation**: Let DeepSeek convert your prompts into ready-to-use diagrams
- **Dockerized Deployment**: Easy setup and consistent environments
- **Time-Saving**: Reduce diagram creation time from hours to minutes

## 🛠️ Quick Start

### Prerequisites
- Docker installed
- Port 8080 available

### Installation & Running
```bash
# Build the Docker image
docker build -t diagram-craft .

# Run the container
docker run -p 8080:8080 --name diagram-craft-instance diagram-craft
