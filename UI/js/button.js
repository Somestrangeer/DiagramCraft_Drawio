const button = document.getElementById('submitButton');

button.addEventListener('click', () => {
   // Добавляем класс для запуска анимации
    button.classList.add('animate1');

    // Удаляем класс после окончания анимации
    button.addEventListener('animationend', () => {
      button.classList.remove('animate1');
    }, { once: true }); // { once: true } чтобы обработчик сработал 1 раз
});