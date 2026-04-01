FROM node:20-alpine
WORKDIR /app
COPY package.json ./
COPY server.js ./
COPY public/ ./public/
EXPOSE 4049
CMD ["node", "server.js"]
