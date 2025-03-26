const particlesConfig = {
    "particles": {
        "number": {
            "value": 60,
            "density": {
                "enable": true,
                "value_area": 800
            }
        },
        "color": {
            "value": "randomColor" // замените на нужный цвет
        },
        "shape": {
            "type": "circle",
            "stroke": {
                "width": 0,
                "color": "#000000"
            },
            "polygon": {
                "nb_sides": 5
            },
            "image": {
                "src": "img/github.svg",
                "width": 100,
                "height": 100
            }
        },
        "opacity": {
            "value": 0.5,
            "random": false,
            "anim": {
                "enable": false,
                "speed": 1,
                "opacity_min": 0.1,
                "sync": false
            }
        },
        "size": {
            "value": 5,
            "random": true,
            "anim": {
                "enable": false,
                "speed": 40,
                "size_min": 0.1,
                "sync": false
            }
        },
        "line_linked": {
            "enable": true,
            "distance": 150,
            "color": "randomColor", // замените на нужный цвет
            "opacity": 0.4,
            "width": 1
        },
        "move": {
            "enable": true,
            "speed": 6,
            "direction": "none",
            "random": false,
            "straight": false,
            "out_mode": "out",
            "attract": {
                "enable": false,
                "rotateX": 600,
                "rotateY": 1200
            }
        }
    },
    "interactivity": {
        "detect_on": "canvas",
        "events": {
            "onhover": {
                "enable": true,
                "mode": "repulse"
            },
            "onclick": {
                "enable": true,
                "mode": "push"
            },
            "resize": true
        },
        "modes": {
            "grab": {
                "distance": 400,
                "line_linked": {
                    "opacity": 1
                }
            },
            "bubble": {
                "distance": 400,
                "size": 40,
                "duration": 2,
                "opacity": 8,
                "speed": 3
            },
            "repulse": {
                "distance": 200
            },
            "push": {
                "particles_nb": 4
            },
            "remove": {
                "particles_nb": 2
            }
        }
    },
    "retina_detect": true,
    "config_demo": {
        "hide_card": false,
        "background_color": "#b61924",
        "background_image": "",
        "background_position": "50% 50%",
        "background_repeat": "no-repeat",
        "background_size": "cover"
    }
};
const iFrameContainer = '<div class="iframeContainer"><iframe id="preview" allowtransparency="true" style="border:5px solid transparent;box-sizing:border-box;position:absolute;width:100%;height:100%; border-radius: 1rem;"></iframe></div>';

document.addEventListener('DOMContentLoaded', function() {
    // Анимация при скролле
    const animateOnScroll = () => {
        const elements = document.querySelectorAll('.animate-fade-up, .animate-fade-in, .animate-slide-in');
        
        elements.forEach((element, index) => {
            const rect = element.getBoundingClientRect();
            const isVisible = rect.top <= window.innerHeight - 100;
            
            if (isVisible) {
                element.style.setProperty('--animation-order', index);
                element.style.visibility = 'visible';
                element.style.animationPlayState = 'running';
            }
        });
    };

    // Инициализация анимаций
    window.addEventListener('scroll', animateOnScroll);
    animateOnScroll(); // Запускаем первую проверку

    // Обработка формы
    const form = document.getElementById('diagramForm');
    if (form) {
        form.addEventListener('submit', function(e) {
            e.preventDefault();
            const toSendData = {
                diagramType: form.diagramType.value,
                prompt: form.prompt.value,
                theme: document.documentElement.getAttribute("data-theme")
            };

            let xhr = new XMLHttpRequest();
            let targetAddress = (form.mode.value == "hand") ? "/"+form.mode.value : "/ai";

            if(targetAddress == "/ai")
            {   
                document.getElementsByClassName("submit-button")[0].disabled = true;
                document.getElementsByClassName("submit-button")[0].style.opacity = "0.5";
                if(document.getElementsByClassName("iframeContainer")[0])
                {
                    document.getElementsByClassName("iframeContainer")[0].remove();
                }
                let preloader = document.getElementById("preloader2");
                let title = document.getElementById("preloadeTitle");
                preloader.style.display = "block";
                title.style.display = "none";

            }

            xhr.open('POST', targetAddress);
            xhr.setRequestHeader('Content-Type', 'application/json; charset=utf-8');
            xhr.onload = function(){
                if(xhr.status === 200)
                {
                    try {
                        const response = JSON.parse(xhr.responseText);
                        console.log('Успешный ответ от сервера', response);
                        if (response.urlDrawio) 
                        {
                            const diagramContainer = document.getElementById('diagram-content');
                            if(document.getElementsByClassName("iframeContainer")[0])
                            {
                                document.getElementsByClassName("iframeContainer")[0].remove();
                            }
                            diagramContainer.innerHTML += iFrameContainer;
                            document.getElementById('preview').src = response.urlDrawio;
                            document.getElementById('preview').style.display = 'block';
                            document.getElementById('preview').parentNode.style.backgroundImage = 'none';

                            if(document.getElementById("preloadeTitle").style.display != "none")
                            { 
                                document.getElementById("preloadeTitle").style.display = "none" 
                            }

                            if(targetAddress == "/ai")
                            {
                                let preloader2 = document.getElementById("preloader2");
                                preloader2.style.display = "none";

                            }
                            document.getElementsByClassName("submit-button")[0].disabled = false;
                            document.getElementsByClassName("submit-button")[0].style.opacity = "1";
                        } 
                        else 
                        { 
                            console.error('В ответе сервера не найдено imageUrl.');
                            document.getElementsByClassName("submit-button")[0].disabled = false;
                            document.getElementsByClassName("submit-button")[0].style.opacity = "1";
                        }
                     } catch (e) {
                        console.error('Ошибка парсинга JSON', e);
                        document.getElementsByClassName("submit-button")[0].disabled = false;
                        document.getElementsByClassName("submit-button")[0].style.opacity = "1";
                     }
                }else {
                    console.error('Ошибка запроса:', xhr.status, xhr.responseText);
                     // Обработка ошибок (например, сообщение об ошибке пользователю)
                     alert('Произошла ошибка при генерации диаграммы. Пожалуйста, попробуйте позже.');
                     document.getElementsByClassName("submit-button")[0].disabled = false;
                     document.getElementsByClassName("submit-button")[0].style.opacity = "1";
                }
            };

            xhr.onerror = function() {
                console.error('Ошибка сети при запросе');
                alert('Ошибка сети. Проверьте подключение к интернету.');
                document.getElementsByClassName("submit-button")[0].disabled = false;
                document.getElementsByClassName("submit-button")[0].style.opacity = "1";
            };

            xhr.send(JSON.stringify(toSendData));
            console.log('Форма отправлена', toSendData);

            
            
        
        });
    }

    // Переключение темы
    const themeToggle = document.getElementById('themeToggle');
    const root = document.documentElement;
    
    const savedTheme = localStorage.getItem('theme') || 
                      (window.matchMedia('(prefers-color-scheme: dark)').matches ? 'dark' : 'light');
    
    root.setAttribute('data-theme', savedTheme);
    updateThemeIcon(savedTheme);
    console.log('Initial theme:', savedTheme);

    if (themeToggle) {
        themeToggle.addEventListener('click', function() {
            const currentTheme = root.getAttribute('data-theme');
            const newTheme = currentTheme === 'light' ? 'dark' : 'light';

            themeToggle.classList.add(newTheme);
            themeToggle.classList.remove(currentTheme);

            let color = (newTheme === 'dark') ? '#ffffff': '#6D5D9B';
            
            root.classList.add('theme-transitioning');
            
            console.log('Switching theme from', currentTheme, 'to', newTheme);
            root.setAttribute('data-theme', newTheme);
            localStorage.setItem('theme', newTheme);
            updateThemeIcon(newTheme);

            particlesConfig["particles"]["color"]["value"] = color;
            particlesConfig["particles"]["line_linked"]["color"] = color;

            particlesJS('particles-js', particlesConfig);
            
            setTimeout(() => {
                root.classList.remove('theme-transitioning');
            }, 1000);
        });
    }

    // Обработка кнопки скачивания
    const downloadButton = document.getElementById('downloadButton');

    if (downloadButton) {
        downloadButton.addEventListener('click', function() {
            if (!this.disabled) {
                console.log('Запрос на скачивание файла');
            }
        });
    }

    function updateThemeIcon(theme) {
        if (themeToggle) {
            themeToggle.textContent = theme === 'light' ? '🌙' : '☀️';
            console.log('Theme icon updated for', theme);
        }
    }

    const gridButton = document.getElementById("gridButton");
    const gridSection = document.getElementById('create-section-grid');

    gridButton.addEventListener('click', function(){

        if(gridSection.classList.contains('create-section-grid-vertical'))
        {
            gridSection.classList.remove('create-section-grid-vertical');
            gridSection.classList.add('create-section-grid-horizontal');
            gridButton.innerHTML = 'pinch';
        }
        else
        {
            gridSection.classList.add('create-section-grid-vertical');
            gridSection.classList.remove('create-section-grid-horizontal');
            gridButton.innerHTML = 'hide';
        }

    });

    let btns = document.getElementsByClassName('copy-button');
    for (let button of btns)
    {
        button.addEventListener('click', (event) => { 
        
        let className = button.classList[1];
        let classArray = className.split('-');
        //console.log(classArray[0]);

        let code = document.getElementById(classArray[0]+'code');console.log(code);

        let tempTextArea = document.createElement('textarea');
        tempTextArea.value = code.innerText; // Копируем текст из блока кода
        document.body.appendChild(tempTextArea);
        
        // Выделяем текст
        tempTextArea.select();
        tempTextArea.setSelectionRange(0, 99999); // Для мобильных устройств
        
        // Копируем текст в буфер обмена
        document.execCommand('copy');
        
        // Удаляем временный элемент
        document.body.removeChild(tempTextArea);

        const notification = document.getElementById('notification');
        notification.classList.add('show');

        // Через 10 секунд скрываем уведомление
        setTimeout(() => {
            notification.classList.remove('show');
        }, 1400); // 10000 миллисекунд = 10 секунд

    
});
    }


});