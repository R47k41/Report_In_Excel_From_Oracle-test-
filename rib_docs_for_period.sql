select
  rownum as "№ п./п.",
  OP.DATEOPER as "Дата проводки",
  OP.DEBACC || OP.DEBCUR || OP.DEBSUB as "Счет Дт",
  OP.KRDACC || OP.KRDCUR || OP.KRDSUB as "Счет Кт",
  OP.SUMMAD as "Сумма в вал. счета по Дт.",
  OP.SUMMAK as "Сумма в вал. счета по Кт.",
  OP.COMMENTS as "Назначение",
  OP.DOCNUMB as "Номер документа(из базы)",
  OP.SUMMAB as "Сумма, руб.",
  (select D.SUMMA from Admin.Docs d where D.ID = OP.DOCSID) as "Сумма"
from
  ADMIN.OPERBOOK op
where
  OP.AUTOOPER = 0
order by
  OP.DATEOPER	
;