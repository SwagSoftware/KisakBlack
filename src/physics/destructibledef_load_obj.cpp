#include "destructibledef_load_obj.h"
#include <xanim/xmodel.h>
#include <universal/com_files.h>
#include <qcommon/common.h>
#include <universal/com_memory.h>
#include <clientscript/cscr_stringlist.h>
#include <EffectsCore/fx_load_obj.h>
#include <game_mp/g_main_mp.h>
#include <xanim/xmodel_load_obj.h>
#include <xanim/xmodel_utils.h>

cspField_t destructiblePieceFields[70] =
{
  { "showBone", 0, 14 },
  { "breakHealth", 4, 7 },
  { "maxTime", 8, 7 },
  { "explosiveDamageOnly", 12, 15 },
  { "meleeDamage", 12, 20 },
  { "parentDamage", 12, 25 },
  { "physicsParent", 12, 30 },
  { "breakEffect", 16, 9 },
  { "breakSound", 20, 0 },
  { "loopSound", 28, 0 },
  { "breakNotify", 24, 0 },
  { "spawnModel1", 32, 10 },
  { "spawnModel2", 36, 10 },
  { "physPreset", 44, 13 },
  { "showBoneD1", 48, 14 },
  { "breakHealthD1", 52, 7 },
  { "maxTimeD1", 56, 7 },
  { "explosiveDamageOnlyD1", 60, 16 },
  { "meleeDamageD1", 60, 21 },
  { "parentDamageD1", 60, 26 },
  { "physicsParentD1", 60, 31 },
  { "breakEffectD1", 64, 9 },
  { "breakSoundD1", 68, 0 },
  { "loopSoundD1", 76, 0 },
  { "breakNotifyD1", 72, 0 },
  { "spawnModel1D1", 80, 10 },
  { "spawnModel2D1", 84, 10 },
  { "physPresetD1", 92, 13 },
  { "showBoneD2", 96, 14 },
  { "breakHealthD2", 100, 7 },
  { "maxTimeD2", 104, 7 },
  { "explosiveDamageOnlyD2", 108, 17 },
  { "meleeDamageD2", 108, 22 },
  { "parentDamageD2", 108, 27 },
  { "physicsParentD2", 108, 32 },
  { "breakEffectD2", 112, 9 },
  { "breakSoundD2", 116, 0 },
  { "loopSoundD2", 124, 0 },
  { "breakNotifyD2", 120, 0 },
  { "spawnModel1D2", 128, 10 },
  { "spawnModel2D2", 132, 10 },
  { "physPresetD2", 140, 13 },
  { "showBoneD3", 144, 14 },
  { "breakHealthD3", 148, 7 },
  { "maxTimeD3", 152, 7 },
  { "explosiveDamageOnlyD3", 156, 18 },
  { "meleeDamageD3", 156, 23 },
  { "parentDamageD3", 156, 28 },
  { "physicsParentD3", 156, 33 },
  { "breakEffectD3", 160, 9 },
  { "breakSoundD3", 164, 0 },
  { "loopSoundD3", 172, 0 },
  { "breakNotifyD3", 168, 0 },
  { "spawnModel1D3", 176, 10 },
  { "spawnModel2D3", 180, 10 },
  { "physPresetD3", 188, 13 },
  { "showBoneD4", 192, 14 },
  { "breakHealthD4", 196, 7 },
  { "maxTimeD4", 200, 7 },
  { "explosiveDamageOnlyD4", 204, 19 },
  { "meleeDamageD4", 204, 24 },
  { "parentDamageD4", 204, 29 },
  { "physicsParentD4", 204, 34 },
  { "breakEffectD4", 208, 9 },
  { "breakSoundD4", 212, 0 },
  { "loopSoundD4", 220, 0 },
  { "breakNotifyD4", 216, 0 },
  { "spawnModel1D4", 224, 10 },
  { "spawnModel2D4", 228, 10 },
  { "physPresetD4", 236, 13 }
};

cspField_t destructibleDefFields[453] =
{
  { "clientOnly", 20, 6 },
  { "model", 4, 10 },
  { "pristineModel", 8, 10 },
  { "piece0", 0, 15 },
  { "piece0Health", 0, 47 },
  { "piece1", 0, 16 },
  { "piece1Health", 0, 48 },
  { "piece2", 0, 17 },
  { "piece2Health", 0, 49 },
  { "piece3", 0, 18 },
  { "piece3Health", 0, 50 },
  { "piece3", 0, 18 },
  { "piece3Health", 0, 50 },
  { "piece4", 0, 19 },
  { "piece4Health", 0, 51 },
  { "piece5", 0, 20 },
  { "piece5Health", 0, 52 },
  { "piece6", 0, 21 },
  { "piece6Health", 0, 53 },
  { "piece7", 0, 22 },
  { "piece7Health", 0, 54 },
  { "piece8", 0, 23 },
  { "piece8Health", 0, 55 },
  { "piece9", 0, 24 },
  { "piece9Health", 0, 56 },
  { "piece10", 0, 25 },
  { "piece10Health", 0, 57 },
  { "piece11", 0, 26 },
  { "piece11Health", 0, 58 },
  { "piece12", 0, 27 },
  { "piece12Health", 0, 59 },
  { "piece13", 0, 28 },
  { "piece13Health", 0, 60 },
  { "piece14", 0, 29 },
  { "piece14Health", 0, 61 },
  { "piece15", 0, 30 },
  { "piece15Health", 0, 62 },
  { "piece16", 0, 31 },
  { "piece16Health", 0, 63 },
  { "piece17", 0, 32 },
  { "piece17Health", 0, 64 },
  { "piece18", 0, 33 },
  { "piece18Health", 0, 65 },
  { "piece19", 0, 34 },
  { "piece19Health", 0, 66 },
  { "piece20", 0, 35 },
  { "piece20Health", 0, 67 },
  { "piece21", 0, 36 },
  { "piece21Health", 0, 68 },
  { "piece22", 0, 37 },
  { "piece22Health", 0, 69 },
  { "piece23", 0, 38 },
  { "piece23Health", 0, 70 },
  { "piece24", 0, 39 },
  { "piece24Health", 0, 71 },
  { "piece25", 0, 40 },
  { "piece25Health", 0, 72 },
  { "piece26", 0, 41 },
  { "piece26Health", 0, 73 },
  { "piece27", 0, 42 },
  { "piece27Health", 0, 74 },
  { "piece28", 0, 43 },
  { "piece28Health", 0, 75 },
  { "piece29", 0, 44 },
  { "piece29Health", 0, 76 },
  { "piece30", 0, 45 },
  { "piece30Health", 0, 77 },
  { "piece31", 0, 46 },
  { "piece31Health", 0, 78 },
  { "parentPiece0", 0, 79 },
  { "parentPiece1", 0, 80 },
  { "parentPiece2", 0, 81 },
  { "parentPiece3", 0, 82 },
  { "parentPiece4", 0, 83 },
  { "parentPiece5", 0, 84 },
  { "parentPiece6", 0, 85 },
  { "parentPiece7", 0, 86 },
  { "parentPiece8", 0, 87 },
  { "parentPiece9", 0, 88 },
  { "parentPiece10", 0, 89 },
  { "parentPiece11", 0, 90 },
  { "parentPiece12", 0, 91 },
  { "parentPiece13", 0, 92 },
  { "parentPiece14", 0, 93 },
  { "parentPiece15", 0, 94 },
  { "parentPiece16", 0, 95 },
  { "parentPiece17", 0, 96 },
  { "parentPiece18", 0, 97 },
  { "parentPiece19", 0, 98 },
  { "parentPiece20", 0, 99 },
  { "parentPiece21", 0, 100 },
  { "parentPiece22", 0, 101 },
  { "parentPiece23", 0, 102 },
  { "parentPiece24", 0, 103 },
  { "parentPiece25", 0, 104 },
  { "parentPiece26", 0, 105 },
  { "parentPiece27", 0, 106 },
  { "parentPiece28", 0, 107 },
  { "parentPiece29", 0, 108 },
  { "parentPiece30", 0, 109 },
  { "parentPiece31", 0, 110 },
  { "parentDamage0", 0, 111 },
  { "parentDamage1", 0, 112 },
  { "parentDamage2", 0, 113 },
  { "parentDamage3", 0, 114 },
  { "parentDamage4", 0, 115 },
  { "parentDamage5", 0, 116 },
  { "parentDamage6", 0, 117 },
  { "parentDamage7", 0, 118 },
  { "parentDamage8", 0, 119 },
  { "parentDamage9", 0, 120 },
  { "parentDamage10", 0, 121 },
  { "parentDamage11", 0, 122 },
  { "parentDamage12", 0, 123 },
  { "parentDamage13", 0, 124 },
  { "parentDamage14", 0, 125 },
  { "parentDamage15", 0, 126 },
  { "parentDamage16", 0, 127 },
  { "parentDamage17", 0, 128 },
  { "parentDamage18", 0, 129 },
  { "parentDamage19", 0, 130 },
  { "parentDamage20", 0, 131 },
  { "parentDamage21", 0, 132 },
  { "parentDamage22", 0, 133 },
  { "parentDamage23", 0, 134 },
  { "parentDamage24", 0, 135 },
  { "parentDamage25", 0, 136 },
  { "parentDamage26", 0, 137 },
  { "parentDamage27", 0, 138 },
  { "parentDamage28", 0, 139 },
  { "parentDamage29", 0, 140 },
  { "parentDamage30", 0, 141 },
  { "parentDamage31", 0, 142 },
  { "bulletDamageScale0", 0, 143 },
  { "bulletDamageScale1", 0, 144 },
  { "bulletDamageScale2", 0, 145 },
  { "bulletDamageScale3", 0, 146 },
  { "bulletDamageScale4", 0, 147 },
  { "bulletDamageScale5", 0, 148 },
  { "bulletDamageScale6", 0, 149 },
  { "bulletDamageScale7", 0, 150 },
  { "bulletDamageScale8", 0, 151 },
  { "bulletDamageScale9", 0, 152 },
  { "bulletDamageScale10", 0, 153 },
  { "bulletDamageScale11", 0, 154 },
  { "bulletDamageScale12", 0, 155 },
  { "bulletDamageScale13", 0, 156 },
  { "bulletDamageScale14", 0, 157 },
  { "bulletDamageScale15", 0, 158 },
  { "bulletDamageScale16", 0, 159 },
  { "bulletDamageScale17", 0, 160 },
  { "bulletDamageScale18", 0, 161 },
  { "bulletDamageScale19", 0, 162 },
  { "bulletDamageScale20", 0, 163 },
  { "bulletDamageScale21", 0, 164 },
  { "bulletDamageScale22", 0, 165 },
  { "bulletDamageScale23", 0, 166 },
  { "bulletDamageScale24", 0, 167 },
  { "bulletDamageScale25", 0, 168 },
  { "bulletDamageScale26", 0, 169 },
  { "bulletDamageScale27", 0, 170 },
  { "bulletDamageScale28", 0, 171 },
  { "bulletDamageScale29", 0, 172 },
  { "bulletDamageScale30", 0, 173 },
  { "bulletDamageScale31", 0, 174 },
  { "explosiveDamageScale0", 0, 175 },
  { "explosiveDamageScale1", 0, 176 },
  { "explosiveDamageScale2", 0, 177 },
  { "explosiveDamageScale3", 0, 178 },
  { "explosiveDamageScale4", 0, 179 },
  { "explosiveDamageScale5", 0, 180 },
  { "explosiveDamageScale6", 0, 181 },
  { "explosiveDamageScale7", 0, 182 },
  { "explosiveDamageScale8", 0, 183 },
  { "explosiveDamageScale9", 0, 184 },
  { "explosiveDamageScale10", 0, 185 },
  { "explosiveDamageScale11", 0, 186 },
  { "explosiveDamageScale12", 0, 187 },
  { "explosiveDamageScale13", 0, 188 },
  { "explosiveDamageScale14", 0, 189 },
  { "explosiveDamageScale15", 0, 190 },
  { "explosiveDamageScale16", 0, 191 },
  { "explosiveDamageScale17", 0, 192 },
  { "explosiveDamageScale18", 0, 193 },
  { "explosiveDamageScale19", 0, 194 },
  { "explosiveDamageScale20", 0, 195 },
  { "explosiveDamageScale21", 0, 196 },
  { "explosiveDamageScale22", 0, 197 },
  { "explosiveDamageScale23", 0, 198 },
  { "explosiveDamageScale24", 0, 199 },
  { "explosiveDamageScale25", 0, 200 },
  { "explosiveDamageScale26", 0, 201 },
  { "explosiveDamageScale27", 0, 202 },
  { "explosiveDamageScale28", 0, 203 },
  { "explosiveDamageScale29", 0, 204 },
  { "explosiveDamageScale30", 0, 205 },
  { "explosiveDamageScale31", 0, 206 },
  { "meleeDamageScale0", 0, 207 },
  { "meleeDamageScale1", 0, 208 },
  { "meleeDamageScale2", 0, 209 },
  { "meleeDamageScale3", 0, 210 },
  { "meleeDamageScale4", 0, 211 },
  { "meleeDamageScale5", 0, 212 },
  { "meleeDamageScale6", 0, 213 },
  { "meleeDamageScale7", 0, 214 },
  { "meleeDamageScale8", 0, 215 },
  { "meleeDamageScale9", 0, 216 },
  { "meleeDamageScale10", 0, 217 },
  { "meleeDamageScale11", 0, 218 },
  { "meleeDamageScale12", 0, 219 },
  { "meleeDamageScale13", 0, 220 },
  { "meleeDamageScale14", 0, 221 },
  { "meleeDamageScale15", 0, 222 },
  { "meleeDamageScale16", 0, 223 },
  { "meleeDamageScale17", 0, 224 },
  { "meleeDamageScale18", 0, 225 },
  { "meleeDamageScale19", 0, 226 },
  { "meleeDamageScale20", 0, 227 },
  { "meleeDamageScale21", 0, 228 },
  { "meleeDamageScale22", 0, 229 },
  { "meleeDamageScale23", 0, 230 },
  { "meleeDamageScale24", 0, 231 },
  { "meleeDamageScale25", 0, 232 },
  { "meleeDamageScale26", 0, 233 },
  { "meleeDamageScale27", 0, 234 },
  { "meleeDamageScale28", 0, 235 },
  { "meleeDamageScale29", 0, 236 },
  { "meleeDamageScale30", 0, 237 },
  { "meleeDamageScale31", 0, 238 },
  { "impactDamageScale0", 0, 239 },
  { "impactDamageScale1", 0, 240 },
  { "impactDamageScale2", 0, 241 },
  { "impactDamageScale3", 0, 242 },
  { "impactDamageScale4", 0, 243 },
  { "impactDamageScale5", 0, 244 },
  { "impactDamageScale6", 0, 245 },
  { "impactDamageScale7", 0, 246 },
  { "impactDamageScale8", 0, 247 },
  { "impactDamageScale9", 0, 248 },
  { "impactDamageScale10", 0, 249 },
  { "impactDamageScale11", 0, 250 },
  { "impactDamageScale12", 0, 251 },
  { "impactDamageScale13", 0, 252 },
  { "impactDamageScale14", 0, 253 },
  { "impactDamageScale15", 0, 254 },
  { "impactDamageScale16", 0, 255 },
  { "impactDamageScale17", 0, 256 },
  { "impactDamageScale18", 0, 257 },
  { "impactDamageScale19", 0, 258 },
  { "impactDamageScale20", 0, 259 },
  { "impactDamageScale21", 0, 260 },
  { "impactDamageScale22", 0, 261 },
  { "impactDamageScale23", 0, 262 },
  { "impactDamageScale24", 0, 263 },
  { "impactDamageScale25", 0, 264 },
  { "impactDamageScale26", 0, 265 },
  { "impactDamageScale27", 0, 266 },
  { "impactDamageScale28", 0, 267 },
  { "impactDamageScale29", 0, 268 },
  { "impactDamageScale30", 0, 269 },
  { "impactDamageScale31", 0, 270 },
  { "physConstraint0", 0, 271 },
  { "physConstraint1", 0, 272 },
  { "physConstraint2", 0, 273 },
  { "physConstraint3", 0, 274 },
  { "physConstraint4", 0, 275 },
  { "physConstraint5", 0, 276 },
  { "physConstraint6", 0, 277 },
  { "physConstraint7", 0, 278 },
  { "physConstraint8", 0, 279 },
  { "physConstraint9", 0, 280 },
  { "physConstraint10", 0, 281 },
  { "physConstraint11", 0, 282 },
  { "physConstraint12", 0, 283 },
  { "physConstraint13", 0, 284 },
  { "physConstraint14", 0, 285 },
  { "physConstraint15", 0, 286 },
  { "physConstraint16", 0, 287 },
  { "physConstraint17", 0, 288 },
  { "physConstraint18", 0, 289 },
  { "physConstraint19", 0, 290 },
  { "physConstraint20", 0, 291 },
  { "physConstraint21", 0, 292 },
  { "physConstraint22", 0, 293 },
  { "physConstraint23", 0, 294 },
  { "physConstraint24", 0, 295 },
  { "physConstraint25", 0, 296 },
  { "physConstraint26", 0, 297 },
  { "physConstraint27", 0, 298 },
  { "physConstraint28", 0, 299 },
  { "physConstraint29", 0, 300 },
  { "physConstraint30", 0, 301 },
  { "physConstraint31", 0, 302 },
  { "damageSound0", 0, 303 },
  { "damageSound1", 0, 304 },
  { "damageSound2", 0, 305 },
  { "damageSound3", 0, 306 },
  { "damageSound4", 0, 307 },
  { "damageSound5", 0, 308 },
  { "damageSound6", 0, 309 },
  { "damageSound7", 0, 310 },
  { "damageSound8", 0, 311 },
  { "damageSound9", 0, 312 },
  { "damageSound10", 0, 313 },
  { "damageSound11", 0, 314 },
  { "damageSound12", 0, 315 },
  { "damageSound13", 0, 316 },
  { "damageSound14", 0, 317 },
  { "damageSound15", 0, 318 },
  { "damageSound16", 0, 319 },
  { "damageSound17", 0, 320 },
  { "damageSound18", 0, 321 },
  { "damageSound19", 0, 322 },
  { "damageSound20", 0, 323 },
  { "damageSound21", 0, 324 },
  { "damageSound22", 0, 325 },
  { "damageSound23", 0, 326 },
  { "damageSound24", 0, 327 },
  { "damageSound25", 0, 328 },
  { "damageSound26", 0, 329 },
  { "damageSound27", 0, 330 },
  { "damageSound28", 0, 331 },
  { "damageSound29", 0, 332 },
  { "damageSound30", 0, 333 },
  { "damageSound31", 0, 334 },
  { "burnEffect0", 0, 335 },
  { "burnEffect1", 0, 336 },
  { "burnEffect2", 0, 337 },
  { "burnEffect3", 0, 338 },
  { "burnEffect4", 0, 339 },
  { "burnEffect5", 0, 340 },
  { "burnEffect6", 0, 341 },
  { "burnEffect7", 0, 342 },
  { "burnEffect8", 0, 343 },
  { "burnEffect9", 0, 344 },
  { "burnEffect10", 0, 345 },
  { "burnEffect11", 0, 346 },
  { "burnEffect12", 0, 347 },
  { "burnEffect13", 0, 348 },
  { "burnEffect14", 0, 349 },
  { "burnEffect15", 0, 350 },
  { "burnEffect16", 0, 351 },
  { "burnEffect17", 0, 352 },
  { "burnEffect18", 0, 353 },
  { "burnEffect19", 0, 354 },
  { "burnEffect20", 0, 355 },
  { "burnEffect21", 0, 356 },
  { "burnEffect22", 0, 357 },
  { "burnEffect23", 0, 358 },
  { "burnEffect24", 0, 359 },
  { "burnEffect25", 0, 360 },
  { "burnEffect26", 0, 361 },
  { "burnEffect27", 0, 362 },
  { "burnEffect28", 0, 363 },
  { "burnEffect29", 0, 364 },
  { "burnEffect30", 0, 365 },
  { "burnEffect31", 0, 366 },
  { "burnSound0", 0, 367 },
  { "burnSound1", 0, 368 },
  { "burnSound2", 0, 369 },
  { "burnSound3", 0, 370 },
  { "burnSound4", 0, 371 },
  { "burnSound5", 0, 372 },
  { "burnSound6", 0, 373 },
  { "burnSound7", 0, 374 },
  { "burnSound8", 0, 375 },
  { "burnSound9", 0, 376 },
  { "burnSound10", 0, 377 },
  { "burnSound11", 0, 378 },
  { "burnSound12", 0, 379 },
  { "burnSound13", 0, 380 },
  { "burnSound14", 0, 381 },
  { "burnSound15", 0, 382 },
  { "burnSound16", 0, 383 },
  { "burnSound17", 0, 384 },
  { "burnSound18", 0, 385 },
  { "burnSound19", 0, 386 },
  { "burnSound20", 0, 387 },
  { "burnSound21", 0, 388 },
  { "burnSound22", 0, 389 },
  { "burnSound23", 0, 390 },
  { "burnSound24", 0, 391 },
  { "burnSound25", 0, 392 },
  { "burnSound26", 0, 393 },
  { "burnSound27", 0, 394 },
  { "burnSound28", 0, 395 },
  { "burnSound29", 0, 396 },
  { "burnSound30", 0, 397 },
  { "burnSound31", 0, 398 },
  { "damageEntity0", 0, 399 },
  { "damageEntity1", 0, 400 },
  { "damageEntity2", 0, 401 },
  { "damageEntity3", 0, 402 },
  { "damageEntity4", 0, 403 },
  { "damageEntity5", 0, 404 },
  { "damageEntity6", 0, 405 },
  { "damageEntity7", 0, 406 },
  { "damageEntity8", 0, 407 },
  { "damageEntity9", 0, 408 },
  { "damageEntity10", 0, 409 },
  { "damageEntity11", 0, 410 },
  { "damageEntity12", 0, 411 },
  { "damageEntity13", 0, 412 },
  { "damageEntity14", 0, 413 },
  { "damageEntity15", 0, 414 },
  { "damageEntity16", 0, 415 },
  { "damageEntity17", 0, 416 },
  { "damageEntity18", 0, 417 },
  { "damageEntity19", 0, 418 },
  { "damageEntity20", 0, 419 },
  { "damageEntity21", 0, 420 },
  { "damageEntity22", 0, 421 },
  { "damageEntity23", 0, 422 },
  { "damageEntity24", 0, 423 },
  { "damageEntity25", 0, 424 },
  { "damageEntity26", 0, 425 },
  { "damageEntity27", 0, 426 },
  { "damageEntity28", 0, 427 },
  { "damageEntity29", 0, 428 },
  { "damageEntity30", 0, 429 },
  { "damageEntity31", 0, 430 },
  { "pieceLabel0", 0, 431 },
  { "pieceLabel1", 0, 432 },
  { "pieceLabel2", 0, 433 },
  { "pieceLabel3", 0, 434 },
  { "pieceLabel4", 0, 435 },
  { "pieceLabel5", 0, 436 },
  { "pieceLabel6", 0, 437 },
  { "pieceLabel7", 0, 438 },
  { "pieceLabel8", 0, 439 },
  { "pieceLabel9", 0, 440 },
  { "pieceLabel10", 0, 441 },
  { "pieceLabel11", 0, 442 },
  { "pieceLabel12", 0, 443 },
  { "pieceLabel13", 0, 444 },
  { "pieceLabel14", 0, 445 },
  { "pieceLabel15", 0, 446 },
  { "pieceLabel16", 0, 447 },
  { "pieceLabel17", 0, 448 },
  { "pieceLabel18", 0, 449 },
  { "pieceLabel19", 0, 450 },
  { "pieceLabel20", 0, 451 },
  { "pieceLabel21", 0, 452 },
  { "pieceLabel22", 0, 453 },
  { "pieceLabel23", 0, 454 },
  { "pieceLabel24", 0, 455 },
  { "pieceLabel25", 0, 456 },
  { "pieceLabel26", 0, 457 },
  { "pieceLabel27", 0, 458 },
  { "pieceLabel28", 0, 459 },
  { "pieceLabel29", 0, 460 },
  { "pieceLabel30", 0, 461 },
  { "pieceLabel31", 0, 462 }
};

bool __cdecl DestructiblePiece_ParseDefSpecificFieldType(unsigned __int8 *pStruct, const char *pValue, int iFieldType)
{
    if ( DestructiblePiece_SetFlag((DestructiblePiece *)pStruct, pValue, iFieldType, 15, 19, 1) )
        return 1;
    if ( DestructiblePiece_SetFlag((DestructiblePiece *)pStruct, pValue, iFieldType, 20, 24, 8) )
        return 1;
    if ( DestructiblePiece_SetFlag((DestructiblePiece *)pStruct, pValue, iFieldType, 25, 29, 2) )
        return 1;
    return DestructiblePiece_SetFlag((DestructiblePiece *)pStruct, pValue, iFieldType, 30, 34, 4) != 0;
}

int __cdecl DestructiblePiece_SetFlag(
                DestructiblePiece *piece,
                const char *pValue,
                int iFieldType,
                int minVal,
                int maxVal,
                int flag)
{
    if ( iFieldType < minVal || iFieldType > maxVal )
        return 0;
    if ( atoi(pValue) )
        piece->stages[iFieldType - minVal].flags |= flag;
    return 1;
}

DestructiblePiece *__cdecl DestructiblePieceLoadFile(const char *name, DestructiblePiece *destructiblePiece)
{
    char dest[128]; // [esp+20h] [ebp-4098h] BYREF
    char buffer[16388]; // [esp+A0h] [ebp-4018h] BYREF
    const char *last; // [esp+40A8h] [ebp-10h]
    signed int v6; // [esp+40ACh] [ebp-Ch]
    int f; // [esp+40B0h] [ebp-8h] BYREF
    int len; // [esp+40B4h] [ebp-4h]

    last = "DESTRUCTIBLEPIECE";
    len = strlen("DESTRUCTIBLEPIECE");
    if ( !strlen(name) )
        return 0;
    if ( Com_sprintf(dest, 0x80u, "destructiblepiece/%s", name) >= 0 )
    {
        v6 = FS_FOpenFileByMode(dest, &f, FS_READ);
        if ( v6 >= 0 )
        {
            FS_Read((unsigned __int8 *)buffer, len, f);
            buffer[len] = 0;
            if ( !strncmp(buffer, last, len) )
            {
                if ( v6 - len < 0x4000 )
                {
                    FS_Read((unsigned __int8 *)buffer, v6 - len, f);
                    buffer[v6 - len] = 0;
                    FS_FCloseFile(f);
                    if ( Info_Validate(buffer) )
                    {
                        if ( !destructiblePiece
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\physics\\destructibledef_load_obj.cpp",
                                        808,
                                        0,
                                        "%s",
                                        "destructiblePiece") )
                        {
                            __debugbreak();
                        }
                        memset(destructiblePiece, 0, sizeof(DestructiblePiece));
                        if ( ParseConfigStringToStruct(
                                     (unsigned __int8 *)destructiblePiece,
                                     destructiblePieceFields,
                                     70,
                                     buffer,
                                     35,
                                     (int (__cdecl *)(unsigned __int8 *, const char *, const int, const int))DestructiblePiece_ParseDefSpecificFieldType,
                                     DestructibleDef_Strcpy) )
                        {
                            return destructiblePiece;
                        }
                        else
                        {
                            return 0;
                        }
                    }
                    else
                    {
                        Com_PrintError(20, "ERROR: destructible piece file [%s] is not valid\n", name);
                        return 0;
                    }
                }
                else
                {
                    Com_PrintError(20, "ERROR: destructible piece file [%s] is to big\n", name);
                    FS_FCloseFile(f);
                    return 0;
                }
            }
            else
            {
                Com_PrintError(20, "ERROR: file [%s] is not a destructible piece file\n", name);
                FS_FCloseFile(f);
                return 0;
            }
        }
        else
        {
            Com_PrintError(20, "ERROR: destructible piece '%s' not found\n", name);
            return 0;
        }
    }
    else
    {
        Com_PrintError(20, "ERROR: filename '%s' too long\n", dest);
        return 0;
    }
}

void __cdecl DestructibleDef_Strcpy(unsigned __int8 *member, const char *keyValue)
{
    char v2; // [esp+3h] [ebp-25h]
    unsigned __int8 *v3; // [esp+8h] [ebp-20h]
    const char *v4; // [esp+Ch] [ebp-1Ch]
    unsigned __int8 *buf; // [esp+20h] [ebp-8h]

    if ( *keyValue )
    {
        buf = Hunk_AllocLowAlign(strlen(keyValue) + 1, 1, "SetDestructibleDefString", 0);
        v4 = keyValue;
        v3 = buf;
        do
        {
            v2 = *v4;
            *v3++ = *v4++;
        }
        while ( v2 );
        *(unsigned int *)member = (unsigned int)buf;
    }
    else
    {
        *(unsigned int *)member = (unsigned int)"";
    }
}

int __cdecl Destructible_ParseDefSpecificFieldType(unsigned __int8 *pStruct, char *pValue, int iFieldType)
{
    float v4; // [esp+0h] [ebp-50h]
    float v5; // [esp+4h] [ebp-4Ch]
    float v6; // [esp+8h] [ebp-48h]
    float v7; // [esp+Ch] [ebp-44h]
    int v8; // [esp+3Ch] [ebp-14h]

    if ( *((unsigned int *)pStruct + 4) )
    {
        if ( iFieldType < 15 || iFieldType > 46 )
        {
            if ( iFieldType < 47 || iFieldType > 78 )
            {
                if ( iFieldType < 79 || iFieldType > 110 )
                {
                    if ( iFieldType < 111 || iFieldType > 142 )
                    {
                        if ( iFieldType < 143 || iFieldType > 174 )
                        {
                            if ( iFieldType < 175 || iFieldType > 206 )
                            {
                                if ( iFieldType < 207 || iFieldType > 238 )
                                {
                                    if ( iFieldType < 239 || iFieldType > 270 )
                                    {
                                        if ( iFieldType < 271 || iFieldType > 302 )
                                        {
                                            if ( iFieldType < 303 || iFieldType > 334 )
                                            {
                                                if ( iFieldType < 335 || iFieldType > 366 )
                                                {
                                                    if ( iFieldType < 367 || iFieldType > 398 )
                                                    {
                                                        if ( iFieldType < 399 || iFieldType > 430 )
                                                        {
                                                            if ( iFieldType >= 431 && iFieldType <= 462 && iFieldType - 431 < *((unsigned int *)pStruct + 3) )
                                                                *(_WORD *)(*((unsigned int *)pStruct + 4) + 312 * (iFieldType - 431) + 288) = SL_GetLowercaseString(pValue, 0, SCRIPTINSTANCE_SERVER);
                                                        }
                                                        else if ( iFieldType - 399 < *((unsigned int *)pStruct + 3) )
                                                        {
                                                            *(float *)(*((unsigned int *)pStruct + 4) + 312 * (iFieldType - 399) + 264) = (float)atoi(pValue) * 0.0099999998;
                                                        }
                                                    }
                                                    else if ( iFieldType - 367 < *((unsigned int *)pStruct + 3) )
                                                    {
                                                        DestructibleDef_Strcpy(
                                                            (unsigned __int8 *)(*((unsigned int *)pStruct + 4) + 312 * (iFieldType - 367) + 284),
                                                            pValue);
                                                    }
                                                }
                                                else if ( iFieldType - 335 < *((unsigned int *)pStruct + 3) )
                                                {
                                                    *(unsigned int *)(*((unsigned int *)pStruct + 4) + 312 * (iFieldType - 335) + 280) = (unsigned int)FX_Register(pValue);
                                                }
                                            }
                                            else if ( !G_ExitAfterToolComplete() && iFieldType - 303 < *((unsigned int *)pStruct + 3) )
                                            {
                                                DestructibleDef_Strcpy(
                                                    (unsigned __int8 *)(*((unsigned int *)pStruct + 4) + 312 * (iFieldType - 303) + 276),
                                                    pValue);
                                            }
                                        }
                                        else if ( iFieldType - 271 < *((unsigned int *)pStruct + 3) && pValue && *pValue )
                                        {
                                            *(unsigned int *)(*((unsigned int *)pStruct + 4) + 312 * (iFieldType - 271) + 268) = (unsigned int)XModel_PhysConstraintsPrecache(pValue, 0);
                                        }
                                    }
                                    else if ( iFieldType - 239 < *((unsigned int *)pStruct + 3) )
                                    {
                                        v4 = atof(pValue);
                                        *(float *)(*((unsigned int *)pStruct + 4) + 312 * (iFieldType - 239) + 260) = v4;
                                    }
                                }
                                else if ( iFieldType - 207 < *((unsigned int *)pStruct + 3) )
                                {
                                    v5 = atof(pValue);
                                    *(float *)(*((unsigned int *)pStruct + 4) + 312 * (iFieldType - 207) + 256) = v5;
                                }
                            }
                            else if ( iFieldType - 175 < *((unsigned int *)pStruct + 3) )
                            {
                                v6 = atof(pValue);
                                *(float *)(*((unsigned int *)pStruct + 4) + 312 * (iFieldType - 175) + 252) = v6;
                            }
                        }
                        else if ( iFieldType - 143 < *((unsigned int *)pStruct + 3) )
                        {
                            v7 = atof(pValue);
                            *(float *)(*((unsigned int *)pStruct + 4) + 312 * (iFieldType - 143) + 248) = v7;
                        }
                    }
                    else if ( iFieldType - 111 < *((unsigned int *)pStruct + 3) )
                    {
                        *(float *)(*((unsigned int *)pStruct + 4) + 312 * (iFieldType - 111) + 244) = (float)atoi(pValue) / 100.0;
                    }
                }
                else
                {
                    v8 = iFieldType - 79;
                    if ( iFieldType - 79 < *((unsigned int *)pStruct + 3) )
                    {
                        *(_BYTE *)(*((unsigned int *)pStruct + 4) + 312 * v8 + 240) = atoi(pValue);
                        if ( *(unsigned __int8 *)(*((unsigned int *)pStruct + 4) + 312 * v8 + 240) != 255
                            && *(unsigned __int8 *)(*((unsigned int *)pStruct + 4) + 312 * v8 + 240) >= *((int *)pStruct + 3)
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\physics\\destructibledef_load_obj.cpp",
                                        854,
                                        0,
                                        "%s",
                                        "ddef->pieces[pieceIndex].parentPiece < ddef->numPieces") )
                        {
                            __debugbreak();
                        }
                    }
                }
            }
            else if ( iFieldType - 47 < *((unsigned int *)pStruct + 3) )
            {
                *(unsigned int *)(*((unsigned int *)pStruct + 4) + 312 * (iFieldType - 47) + 272) = atoi(pValue);
            }
        }
        else
        {
            DestructiblePieceLoadFile(pValue, (DestructiblePiece *)(*((unsigned int *)pStruct + 4) + 312 * (iFieldType - 15)));
        }
        return 1;
    }
    else
    {
        if ( iFieldType >= 15 && iFieldType <= 46 && iFieldType - 15 + 1 > *((unsigned int *)pStruct + 3) )
            *((unsigned int *)pStruct + 3) = iFieldType - 15 + 1;
        return 1;
    }
}

DestructibleDef *__cdecl DestructibleDefLoadFile(const char *name, void *(__cdecl *Alloc)(int))
{
    unsigned __int8 *v3; // eax
    void *v4; // eax
    char *v5; // eax
    char *v6; // eax
    const char *v7; // [esp-14h] [ebp-80A8h]
    const char *v8; // [esp-Ch] [ebp-80A0h]
    const char *v9; // [esp-Ch] [ebp-80A0h]
    int v10; // [esp-8h] [ebp-809Ch]
    int v11; // [esp-8h] [ebp-809Ch]
    int v12; // [esp-4h] [ebp-8098h]
    int v13; // [esp-4h] [ebp-8098h]
    unsigned __int8 index; // [esp+27h] [ebp-806Dh] BYREF
    unsigned int namea; // [esp+28h] [ebp-806Ch]
    int j; // [esp+2Ch] [ebp-8068h]
    int i; // [esp+30h] [ebp-8064h]
    char dest[64]; // [esp+34h] [ebp-8060h] BYREF
    char buffer[32768]; // [esp+74h] [ebp-8020h] BYREF
    const char *last; // [esp+8078h] [ebp-1Ch]
    unsigned __int8 *pStruct; // [esp+807Ch] [ebp-18h]
    unsigned int count; // [esp+8080h] [ebp-14h]
    signed int v23; // [esp+8084h] [ebp-10h]
    int f; // [esp+8088h] [ebp-Ch] BYREF
    XModel *model; // [esp+808Ch] [ebp-8h]
    int len; // [esp+8090h] [ebp-4h]

    last = "DESTRUCTIBLEDEF";
    len = strlen("DESTRUCTIBLEDEF");
    if ( !strlen(name) )
        return 0;
    if ( Com_sprintf(dest, 0x40u, "destructibledef/%s", name) >= 0 )
    {
        v23 = FS_FOpenFileByMode(dest, &f, FS_READ);
        if ( v23 >= 0 )
        {
            FS_Read((unsigned __int8 *)buffer, len, f);
            buffer[len] = 0;
            if ( !strncmp(buffer, last, len) )
            {
                if ( v23 - len < 0x8000 )
                {
                    FS_Read((unsigned __int8 *)buffer, v23 - len, f);
                    buffer[v23 - len] = 0;
                    FS_FCloseFile(f);
                    if ( Info_Validate(buffer) )
                    {
                        pStruct = (unsigned __int8 *)Alloc(24);
                        if ( !pStruct
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\physics\\destructibledef_load_obj.cpp",
                                        995,
                                        0,
                                        "%s",
                                        "destructibleDef") )
                        {
                            __debugbreak();
                        }
                        v3 = pStruct;
                        *(unsigned int *)pStruct = 0;
                        *((unsigned int *)v3 + 1) = 0;
                        *((unsigned int *)v3 + 2) = 0;
                        *((unsigned int *)v3 + 3) = 0;
                        *((unsigned int *)v3 + 4) = 0;
                        *((unsigned int *)v3 + 5) = 0;
                        if ( ParseConfigStringToStruct(
                                     pStruct,
                                     destructibleDefFields,
                                     453,
                                     buffer,
                                     463,
                                     (int (__cdecl *)(unsigned __int8 *, const char *, const int, const int))Destructible_ParseDefSpecificFieldType,
                                     DestructibleDef_Strcpy) )
                        {
                            count = 312 * *((unsigned int *)pStruct + 3);
                            v4 = Alloc(count);
                            *((unsigned int *)pStruct + 4) = (unsigned int)v4;
                            memset(*((unsigned __int8 **)pStruct + 4), 0, count);
                            if ( ParseConfigStringToStruct(
                                         pStruct,
                                         destructibleDefFields,
                                         453,
                                         buffer,
                                         463,
                                         (int (__cdecl *)(unsigned __int8 *, const char *, const int, const int))Destructible_ParseDefSpecificFieldType,
                                         DestructibleDef_Strcpy) )
                            {
                                model = (XModel *)*((unsigned int *)pStruct + 1);
                                if ( model )
                                {
                                    for ( i = 0; i < *((unsigned int *)pStruct + 3); ++i )
                                    {
                                        for ( j = 0; j < 5; ++j )
                                        {
                                            *(float *)(*((unsigned int *)pStruct + 4) + 312 * i + 48 * j + 4) = *(float *)(312 * i
                                                                                                                                                                                         + *((unsigned int *)pStruct + 4)
                                                                                                                                                                                         + 48 * j
                                                                                                                                                                                         + 4)
                                                                                                                                                                    / 100.0;
                                            namea = *(unsigned __int16 *)(312 * i + *((unsigned int *)pStruct + 4) + 48 * j);
                                            if ( namea )
                                            {
                                                index = 0;
                                                if ( XModelGetBoneIndex(model, namea, &index) )
                                                {
                                                    if ( *(unsigned int *)(*((unsigned int *)pStruct + 4) + 312 * i + 48 * j + 44) )
                                                    {
                                                        if ( !XModelGetCollmapForBoneIndex(model, index) )
                                                        {
                                                            v13 = j;
                                                            v11 = i;
                                                            v9 = model->name;
                                                            v7 = **(const char ***)(*((unsigned int *)pStruct + 4) + 312 * i + 48 * j + 44);
                                                            v6 = SL_ConvertToString(namea, SCRIPTINSTANCE_SERVER);
                                                            Com_PrintError(
                                                                20,
                                                                "ERROR: No collmap for bone [%s] but it has a physPreset [%s] in destructible file [%s] m"
                                                                "odel [%s] piece [%i] stage [%i]\n",
                                                                v6,
                                                                v7,
                                                                name,
                                                                v9,
                                                                v11,
                                                                v13);
                                                        }
                                                    }
                                                    *(unsigned int *)(*((unsigned int *)pStruct + 4) + 312 * i + 4 * ((int)index >> 5) + 292) = *(unsigned int *)(312 * i + *((unsigned int *)pStruct + 4) + 4 * ((int)index >> 5) + 292) | (0x80000000 >> (index & 0x1F));
                                                }
                                                else
                                                {
                                                    v12 = j;
                                                    v10 = i;
                                                    v8 = model->name;
                                                    v5 = SL_ConvertToString(namea, SCRIPTINSTANCE_SERVER);
                                                    Com_PrintError(
                                                        20,
                                                        "ERROR: Destructible file [%s] can not find bone [%s] in model [%s] for piece [%i] stage [%i]\n",
                                                        name,
                                                        v5,
                                                        v8,
                                                        v10,
                                                        v12);
                                                }
                                            }
                                        }
                                    }
                                    return (DestructibleDef *)pStruct;
                                }
                                else
                                {
                                    return 0;
                                }
                            }
                            else
                            {
                                return 0;
                            }
                        }
                        else
                        {
                            return 0;
                        }
                    }
                    else
                    {
                        Com_PrintError(20, "ERROR: Destructible file [%s] is not valid\n", name);
                        return 0;
                    }
                }
                else
                {
                    Com_PrintError(20, "ERROR: Destructible file [%s] is too big\n", name);
                    FS_FCloseFile(f);
                    return 0;
                }
            }
            else
            {
                Com_PrintError(20, "ERROR: File [%s] is not a destructible file\n", name);
                FS_FCloseFile(f);
                return 0;
            }
        }
        else
        {
            Com_PrintError(20, "ERROR: Destructible '%s' not found\n", name);
            return 0;
        }
    }
    else
    {
        Com_PrintError(20, "ERROR: Filename '%s' too long\n", dest);
        return 0;
    }
}

int __cdecl XModelGetBoneIndex(const XModel *model, unsigned int name, unsigned __int8 *index)
{
    unsigned int numBones; // [esp+0h] [ebp-Ch]
    unsigned int localBoneIndex; // [esp+4h] [ebp-8h]
    unsigned __int16 *boneNames; // [esp+8h] [ebp-4h]

    if ( !index
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\physics\\destructibledef_load_obj.cpp",
                    1082,
                    0,
                    "%s",
                    "index") )
    {
        __debugbreak();
    }
    boneNames = model->localBoneNames;
    numBones = model->numBones;

    iassert(numBones < DOBJ_MAX_PARTS);

    for ( localBoneIndex = 0; ; ++localBoneIndex )
    {
        if ( localBoneIndex >= numBones )
            return 0;
        if ( name == boneNames[localBoneIndex] )
            break;
    }
    *index = localBoneIndex;
    if ( *index != localBoneIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\physics\\destructibledef_load_obj.cpp",
                    1094,
                    0,
                    "%s",
                    "*index == localBoneIndex") )
    {
        __debugbreak();
    }
    return 1;
}

