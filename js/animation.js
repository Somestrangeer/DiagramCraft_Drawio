function mapNumberRange(n, a, b, c, d) {
  return ((n - a) * (d - c)) / (b - a) + c
}

function setup() {
  Array.from(document.querySelectorAll('.card')).map((cardEl) =>
    initCard(cardEl)
  )
}
function initCard(card) {
  const cardContent = card.querySelector('.card__content')
  //const gloss = card.querySelector('.card__gloss')

  // requestAnimationFrame(() => {
  //   gloss.classList.add('card__gloss--animatable')
  // })

  card.addEventListener('mousemove', (e) => {
    const pointerX = e.clientX
    const pointerY = e.clientY
    const cardRect = card.getBoundingClientRect()
    const halfWidth = cardRect.width / 2
    const halfHeight = cardRect.height / 2
    const cardCenterX = cardRect.left + halfWidth
    const cardCenterY = cardRect.top + halfHeight
    const deltaX = pointerX - cardCenterX
    const deltaY = pointerY - cardCenterY
    const distanceToCenter = Math.sqrt(deltaX * deltaX + deltaY * deltaY)
    const maxDistance = Math.max(halfWidth, halfHeight)
    const degree = mapNumberRange(distanceToCenter, 0, maxDistance, 0, 2)
    const rx = mapNumberRange(deltaY, 0, halfWidth, 0, 1)
    const ry = mapNumberRange(deltaX, 0, halfHeight, 0, 1)
    cardContent.style.transform = `perspective(400px) rotate3d(${-rx}, ${ry}, 0, ${degree}deg)`

    // gloss.style.transform = `translate(${-ry * 100}%, ${-rx * 100}%) scale(2.4)`

    // gloss.style.opacity = `${mapNumberRange(
    //   distanceToCenter,
    //   0,
    //   maxDistance,
    //   0,
    //   0.6
    // )}`
  })

  card.addEventListener('mouseleave', () => {
    cardContent.style = null
      // gloss.style.opacity = 0
  })
}

setup()
