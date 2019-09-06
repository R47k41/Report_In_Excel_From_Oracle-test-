select
  rownum as "� �./�.",
  OP.DATEOPER as "���� ��������",
  OP.DEBACC || OP.DEBCUR || OP.DEBSUB as "���� ��",
  OP.KRDACC || OP.KRDCUR || OP.KRDSUB as "���� ��",
  OP.SUMMAD as "����� � ���. ����� �� ��.",
  OP.SUMMAK as "����� � ���. ����� �� ��.",
  OP.COMMENTS as "����������",
  OP.DOCNUMB as "����� ���������(�� ����)",
  OP.SUMMAB as "�����, ���.",
  (select D.SUMMA from Admin.Docs d where D.ID = OP.DOCSID) as "�����"
from
  ADMIN.OPERBOOK op
where
  OP.AUTOOPER = 0
order by
  OP.DATEOPER	
;