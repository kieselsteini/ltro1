-- it's better to require ltro1 :)
local ltro = require('ltro1')

--              size     content
local sprite = '1212' .. string.rep('012345678901', 12)


-- callback which is called once upon startup
function ltro.on_init()
    -- Zelda NES dungeon music (uses both audio channels)
    ltro.play(1, 'm2t90o2l16ga+>dd+<ga+>dd+<ga+>dd+<ga+>dd+<f+a>dd+<f+a>dd+<f+a>dd+<f+a>dd+<fg+>dd+<fg+>dd+<fg+>dd+<fg+>dd+<eg>dd+<eg>dd+<eg>dd+<eg>dd+<d+g>cd<d+g>cd<d+g>cd<d+g>cd<dg>cd<dg>cd<dg>cd<dg>cd<cf+ab+f+a>cd+<a>cd+cd+f+d+f+af+ab+g4:')
    ltro.play(2, 'm5t90o2<g2a+4>d4c+4<f+2.f2&fg+4>c+8c4<e2.l16d+dd+4.g8.>d+8.d8<dc+d4.g8.>d8.c+8<df+af+ab+a>cd+cd+f+af+d+cd+c<af+g4:')
end


-- callback which is called once upon shutdown
function ltro.on_quit()
    print('bye bye ...')
end


-- callback which is called for every frame
function ltro.on_tick(counter)
    ltro.draw(sprite, math.random(240) - 1, math.random(135) - 1)
    ltro.rect(0, 0, 0, 240, 7, true)
    ltro.rect(0, 0, 135-8, 240, 135, true)
    ltro.print(9, 0, 0, string.format('mem=%.2fKiB tick=%d', collectgarbage('count'), counter))
    ltro.print(6, 0, 135-8, 'A=Clear B=Silence')

    if ltro.btnp('a') then
        ltro.clear()
    end
    if ltro.btn('b') then
        ltro.stop(1)
        ltro.stop(2)
    end
end
