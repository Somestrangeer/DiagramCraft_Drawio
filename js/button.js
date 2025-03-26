const button = document.getElementById('closeButton');
button.addEventListener('click', function(){
  const popupElements = document.getElementsByClassName("popupLayer");
  if (popupElements.length > 0) 
  {
     popupElements[0].classList.remove('visibleObject');
     popupElements[0].classList.add('hiddenObject');
  }
});