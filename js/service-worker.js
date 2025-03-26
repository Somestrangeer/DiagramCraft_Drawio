self.addEventListener('fetch', event => {
    const url = new URL(event.request.url);
    console.log("434343");
    // Проверяем, является ли запрашиваемый ресурс SVG-файлом
    if (url.pathname.endsWith('.svg')) {
        console.log('Перехвачен запрос на SVG:', event.request.url);
    }

    // Продолжаем обработку запроса
    event.respondWith(fetch(event.request));
});
