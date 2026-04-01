# laserharp  
Browser-based Laser Harp instrument for interactive musical performance and experimentation

## Description  
laserharp is a lightweight web server delivering a browser-based laser harp instrument interface. It serves static assets including an interactive HTML5 canvas UI that simulates a laser harp, enabling users to play and experiment with musical scales and waveforms directly in the browser. This project targets musicians, hobbyists, and developers interested in digital musical instruments and interactive sound synthesis.

## Features  
- **Static file server** serving the laser harp web interface on port 4049  
- **Interactive UI** rendered in `public/index.html` with:  
  - Top control bar featuring:  
    - Scale selector buttons  
    - Waveform selector buttons  
    - Toggle switches for sound options  
  - Canvas-based laser harp visualization and interaction  
  - Bottom key hints for user guidance  
- **Custom fonts and styling** for a sleek dark-themed digital instrument look  
- **Audio playback support** for multiple audio formats (wav, mp3, ogg) as indicated by MIME types  
- **Dockerized deployment** for easy container-based hosting  
- **Single HTTP server** implemented in `server.js` serving all static assets securely with directory traversal protection

## Tech Stack  

| Technology | Role                          |
|------------|-------------------------------|
| Node.js    | HTTP static file server        |
| JavaScript | Server-side scripting          |
| HTML5      | User interface markup          |
| CSS3       | UI styling and layout          |
| Docker     | Containerization and deployment|
| HTTP       | Protocol for serving assets    |

## Architecture  
The project consists of a single Node.js HTTP server (`server.js`) that serves static files from the `public/` directory. The client-side UI is a single-page application contained in `public/index.html` with embedded CSS styles. The server listens on port 4049 and handles requests by mapping URLs to files within the `public/` folder, enforcing security by preventing directory traversal. The Dockerfile builds a lightweight Node.js Alpine Linux container image, exposing port 4049, which is mapped to port 4051 on the host via `docker-compose.yml`.

## Prerequisites  
- Node.js 20.x (tested with Node 20-alpine base image)  
- Docker (for containerized deployment)  
- Docker Compose (for multi-service orchestration, though only one service here)  

## Installation & Setup  

1. Clone the repository:  
   ```bash
   git clone <repository-url>
   cd laserharp
   ```

2. Install dependencies (none explicitly listed, so no `npm install` needed)  

3. To run locally without Docker, use the start script:  
   ```bash
   npm start
   ```

## Running  

### Development / Local  
Run the server directly with Node.js:  
```bash
npm start
```
This starts the HTTP server on `http://localhost:4049`.

### Production / Docker  
Build and run the Docker container:  
```bash
docker build -t laserharp .
docker run -p 4051:4049 --name laserharp --restart unless-stopped laserharp
```

Or use Docker Compose:  
```bash
docker-compose up -d
```
This runs the container named `laserharp` exposing port 4051 on the host mapped to 4049 in the container.

## Docker  
- Build image:  
  ```bash
  docker build -t laserharp .
  ```  
- Run container:  
  ```bash
  docker run -p 4051:4049 --name laserharp --restart unless-stopped laserharp
  ```  
- Using Docker Compose:  
  ```bash
  docker-compose up -d
  ```  
- Stop container:  
  ```bash
  docker-compose down
  ```

## API Overview  
This project does not expose a REST API or backend routes beyond static file serving. The HTTP server serves files from the `public/` directory:  
- `/` → serves `index.html`  
- `/[filename]` → serves static assets such as CSS, JS, images, audio files  

## Environment Variables  

| Variable | Description                    | Required |
|----------|-------------------------------|----------|
| (none)   | No environment variables used | No       |

---

**Note:** The server port is hardcoded to `4049` in `server.js`. To change it, modify the `PORT` constant in the source code.
